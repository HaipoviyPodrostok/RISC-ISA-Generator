#pragma once

#include <nlohmann/json.hpp>

#include "types.hpp"

namespace json_parser {

class JsonParser {
 public:
  explicit JsonParser(const std::string& filename);

  [[nodiscard]] IsaDescription Parse();

 private:
  nlohmann::json data;

  void LenthParsing(IsaDescription& desc) const;
  void FieldsParsing(IsaDescription& desc) const;
  void InstrParsing(IsaDescription& desc) const;
};

}  // namespace json_parser