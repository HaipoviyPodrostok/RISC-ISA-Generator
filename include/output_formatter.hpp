#pragma once

#include <ostream>

#include "types.hpp"

class IOutputFormatter {
 public:
  virtual ~IOutputFormatter()                                              = default;
  virtual void Format(const LayoutResult& result, std::ostream& os) const  = 0;
};

class JsonOutputFormatter : public IOutputFormatter {
 public:
  void Format(const LayoutResult& result, std::ostream& os) const override;
};
