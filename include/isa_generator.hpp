#pragma once

#include "types.hpp"

class IsaGenerator {
 public:
  explicit IsaGenerator(IsaDescription& desc);

 private:
  IsaDescription desc_;

  void Validate() const;
};