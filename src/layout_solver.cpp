#include "layout_solver.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string_view>

namespace {

constexpr std::string_view FieldFormat    = "F";
constexpr std::string_view FieldOpcode    = "OPCODE";
constexpr std::string_view FieldResPrefix = "RES";
constexpr std::string_view ValFlexible    = "+";

std::string ToBinaryString(int value, int bits) {
  std::string s;
  for (int i = bits - 1; i >= 0; --i) {
    s += ((value >> i) & 1) ? '1' : '0';
  }
  return s;
}

}  // namespace

LayoutResult BacktrackingLayoutSolver::Solve(const IsaDescription& desc) {
  desc_ = desc;
  fields_.clear();
  free_bits_.clear();
  layout_.clear();

  InitFields();

  free_bits_.assign(desc_.instructions.size(),
                    std::vector<bool>(desc_.total_length, true));
  layout_.resize(fields_.size());

  if (!FindLayout(0)) {
    throw std::runtime_error("Failed to pack layout into " +
                             std::to_string(desc_.total_length) + " bits.");
  }

  LayoutResult result;
  int num_fmt   = desc_.instructions.size();
  int total_len = desc_.total_length;

  for (int fmt_i = 0; fmt_i < num_fmt; ++fmt_i) {
    const auto&                                    group = desc_.instructions[fmt_i];
    std::vector<std::tuple<int, int, std::string>> chunks;

    for (size_t fi = 0; fi < fields_.size(); ++fi) {
      if (std::find(fields_[fi].formats.begin(), fields_[fi].formats.end(), fmt_i) !=
          fields_[fi].formats.end())
      {
        chunks.push_back({layout_[fi].second, layout_[fi].first, fields_[fi].name});
      }
    }

    int         res_count = 0;
    int         bit       = total_len - 1;
    const auto& fbits     = free_bits_[fmt_i];

    while (bit >= 0) {
      if (fbits[bit]) {
        int msb = bit;
        while (bit >= 0 && fbits[bit])
          bit--;
        int lsb = bit + 1;
        chunks.push_back(
          {msb, lsb, std::string(FieldResPrefix) + std::to_string(res_count++)});
      }
      else {
        bit--;
      }
    }

    std::sort(chunks.begin(), chunks.end(), [](const auto& a, const auto& b) {
      return std::get<0>(a) > std::get<0>(b);
    });

    for (size_t insn_idx = 0; insn_idx < group.insns.size(); ++insn_idx) {
      EncodedInstruction enc_insn;
      enc_insn.insn = group.insns[insn_idx];

      for (const auto& chunk : chunks) {
        int         msb  = std::get<0>(chunk);
        int         lsb  = std::get<1>(chunk);
        std::string name = std::get<2>(chunk);
        std::string val  = std::string(ValFlexible);

        if (name == FieldFormat) {
          val = ToBinaryString(fmt_i, msb - lsb + 1);
        }
        else if (name == FieldOpcode) {
          val = ToBinaryString(insn_idx, msb - lsb + 1);
        }
        else if (name.find(FieldResPrefix) == 0) {
          val = std::string(msb - lsb + 1, '0');
        }

        enc_insn.fields.push_back({msb, lsb, name, val});
      }
      result.push_back(enc_insn);
    }
  }

  return result;
}

void BacktrackingLayoutSolver::InitFields() {
  int num_fmt = desc_.instructions.size();
  int f_bits  = (num_fmt > 1) ? std::ceil(std::log2(num_fmt)) : 0;

  int op_bits = 0;
  for (const auto& group : desc_.instructions) {
    if (group.insns.size() > 1) {
      int bits = std::ceil(std::log2(group.insns.size()));
      if (bits > op_bits)
        op_bits = bits;
    }
  }

  std::map<std::string, FieldMeta> field_map;

  if (f_bits > 0) {
    FieldMeta f {std::string(FieldFormat), f_bits, false, {}};
    for (int i = 0; i < num_fmt; ++i)
      f.formats.push_back(i);
    field_map[std::string(FieldFormat)] = f;
  }

  if (op_bits > 0) {
    FieldMeta op {std::string(FieldOpcode), op_bits, false, {}};
    for (int i = 0; i < num_fmt; ++i) {
      if (desc_.instructions[i].insns.size() > 1)
        op.formats.push_back(i);
    }
    field_map[std::string(FieldOpcode)] = op;
  }

  for (int i = 0; i < num_fmt; ++i) {
    for (const auto& op_name : desc_.instructions[i].operands) {
      if (field_map.find(op_name) == field_map.end()) {
        auto it = desc_.fields.find(op_name);
        if (it == desc_.fields.end())
          throw std::runtime_error("Unknown operand: " + op_name);
        field_map[op_name] = {
          op_name, (int)it->second.min_size, it->second.is_flexible, {}};
      }
      field_map[op_name].formats.push_back(i);
    }
  }

  for (auto& kv : field_map)
    fields_.push_back(kv.second);

  std::sort(fields_.begin(), fields_.end(), [](const FieldMeta& a, const FieldMeta& b) {
    if (a.formats.size() != b.formats.size())
      return a.formats.size() > b.formats.size();
    if (a.flexible != b.flexible)
      return a.flexible < b.flexible;
    return a.min_size > b.min_size;
  });
}

bool BacktrackingLayoutSolver::FindLayout(size_t field_idx) {
  if (field_idx == fields_.size())
    return true;

  const auto& field     = fields_[field_idx];
  int         total_len = desc_.total_length;
  int         min_sz    = field.min_size;
  int         max_sz    = field.flexible ? total_len : min_sz;

  for (int sz = max_sz; sz >= min_sz; --sz) {
    for (int msb = total_len - 1; msb >= sz - 1; --msb) {
      int lsb = msb - sz + 1;

      if (!CanPlace(field, lsb, msb))
        continue;

      TraceBits(field, lsb, msb, false);
      layout_[field_idx] = {lsb, msb};

      if (CheckCapacity(field_idx)) {
        if (FindLayout(field_idx + 1))
          return true;
      }

      TraceBits(field, lsb, msb, true);
    }
  }

  return false;
}

bool BacktrackingLayoutSolver::CanPlace(const FieldMeta& field, int lsb, int msb) const {
  for (int fmt : field.formats) {
    for (int b = lsb; b <= msb; ++b) {
      if (!free_bits_[fmt][b])
        return false;
    }
  }
  return true;
}

void BacktrackingLayoutSolver::TraceBits(const FieldMeta& field, int lsb, int msb, bool free) {
  for (int fmt : field.formats) {
    for (int b = lsb; b <= msb; ++b) {
      free_bits_[fmt][b] = free;
    }
  }
}

bool BacktrackingLayoutSolver::CheckCapacity(size_t field_idx) const {
  int         total_len    = desc_.total_length;
  const auto& placed_field = fields_[field_idx];

  for (int fmt : placed_field.formats) {
    int max_seg = 0, curr_seg = 0, total_free = 0;

    for (int b = 0; b < total_len; ++b) {
      if (free_bits_[fmt][b]) {
        curr_seg++;
        total_free++;
        if (curr_seg > max_seg)
          max_seg = curr_seg;
      }
      else {
        curr_seg = 0;
      }
    }

    int req_total = 0, req_max = 0;
    for (size_t i = field_idx + 1; i < fields_.size(); ++i) {
      const auto& next_field = fields_[i];
      if (std::find(next_field.formats.begin(), next_field.formats.end(), fmt) !=
          next_field.formats.end())
      {
        req_total += next_field.min_size;
        if (next_field.min_size > req_max)
          req_max = next_field.min_size;
      }
    }

    if (req_total > total_free || req_max > max_seg)
      return false;
  }
  return true;
}
