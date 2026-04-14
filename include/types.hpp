#pragma once

#include <map>
#include <string>
#include <vector>

struct FieldConfig {
  std::string name;
  size_t      min_size;
  bool        is_flexible;
};

struct InstrConfig {
  std::vector<std::string> insns;
  std::vector<std::string> operands;
  std::string              format;
  std::string              comment;
};

struct IsaDescription {
  size_t                             total_length;
  std::map<std::string, FieldConfig> fields;
  std::vector<InstrConfig>           instructions;
};

struct FieldAllocation {
  int         msb;
  int         lsb;
  std::string name;
  std::string value;
};

struct EncodedInstruction {
  std::string                  insn;
  std::vector<FieldAllocation> fields;
};

using LayoutResult = std::vector<EncodedInstruction>;
