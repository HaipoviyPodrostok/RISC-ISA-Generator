#include "output_formatter.hpp"

#include <nlohmann/json.hpp>

void JsonOutputFormatter::Format(const LayoutResult& result, std::ostream& os) const {
  nlohmann::json output = nlohmann::json::array();

  for (const auto& enc_insn : result) {
    nlohmann::json insn_json;
    insn_json["insn"] = enc_insn.insn;

    nlohmann::json fields_json = nlohmann::json::array();
    for (const auto& field : enc_insn.fields) {
      nlohmann::json f_obj;
      f_obj[field.name] = {
        {"msb",   field.msb},
        {"lsb",   field.lsb},
        {"value", field.value}
      };
      fields_json.push_back(f_obj);
    }
    
    insn_json["fields"] = fields_json;
    output.push_back(insn_json);
  }

  os << output.dump(2) << "\n";
}
