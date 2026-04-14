#include "json_parser.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "types.hpp"

namespace json_parser {

using json = nlohmann::json;

JsonParser::JsonParser(const std::string& filename) {
  std::ifstream json_file(filename);
  if (!json_file) {
    std::runtime_error("Failed to open " + filename);
  }

  try {
    json_file >> data;
  }
  catch (const json::parse_error& e) {
    throw std::runtime_error(std::string("JSON parse error: ") + e.what());
  }
}

IsaDescription JsonParser::Parse() {
  IsaDescription desc;

  LenthParsing(desc);
  FieldsParsing(desc);
  InstrParsing(desc);

  return desc;
}

void JsonParser::LenthParsing(IsaDescription& desc) const {
  try {
    desc.total_length = std::stoi(data.at("length").get<std::string>());
  }
  catch (const std::exception& e) {
    throw std::runtime_error("Invalid 'length' field.");
  }
}

void JsonParser::FieldsParsing(IsaDescription& desc) const {
  if (!data.contains("fields") || !data["fields"].is_array()) {
    throw std::runtime_error("Invalid 'fields' array.");
  }

  for (const auto& field_item : data["fields"]) {
    for (auto it = field_item.begin(); it != field_item.end(); ++it) {
      FieldConfig field_cfg;

      field_cfg.name       = it.key();
      std::string size_str = it.value().get<std::string>();

      if (size_str.find(">=") != std::string::npos) {
        field_cfg.is_flexible = true;
        field_cfg.min_size    = std::stoi(size_str.substr(2));
      }
      else {
        field_cfg.is_flexible = false;
        field_cfg.min_size    = std::stoi(size_str);
      }
      desc.fields[field_cfg.name] = field_cfg;
    }
  }
}

void JsonParser::InstrParsing(IsaDescription& desc) const {
  if (!data.contains("instructions") || !data["instructions"].is_array()) {
    throw std::runtime_error("Missing or invalid 'instructions' array.");
  }

  for (const auto& inst_group : data["instructions"]) {
    InstrConfig instr_cfg;
    instr_cfg.format = inst_group.at("format").get<std::string>();

    for (const auto& insn : inst_group.at("insns")) {
      instr_cfg.insns.push_back(insn.get<std::string>());
    }

    for (const auto& op : inst_group.at("operands")) {
      std::string op_name = op.get<std::string>();
      instr_cfg.operands.push_back(op_name);

      if (desc.fields.find(op_name) == desc.fields.end()) {
        throw std::runtime_error("Format '" + instr_cfg.format +
                                 "' uses undefined operand: " + op_name);
      }
    }
    desc.instructions.push_back(instr_cfg);
  }
}
}  // namespace json_parser
