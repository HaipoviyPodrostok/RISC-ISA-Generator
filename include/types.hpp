#pragma once

#include <string>
#include <vector>

enum class ConstraintType {
  Exact,
  Minimum
};

struct FieldConfig {
  std::string name;
  int bits;
  ConstraintType constraint;
};

struct InstructionConfig {
  std::vector<std::string> names;
  std::vector<std::string> operands;
  std::string format;
  std::string comment;
};

struct RawConfig {
  int inst_len;
  std::vector<FieldConfig>       fields;
  std::vector<InstructionConfig> instructions;
};