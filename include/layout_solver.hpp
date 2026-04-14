#pragma once

#include <string>
#include <utility>
#include <vector>

#include "types.hpp"

class ILayoutSolver {
 public:
  virtual ~ILayoutSolver()                               = default;
  virtual LayoutResult Solve(const IsaDescription& desc) = 0;
};

class BacktrackingLayoutSolver : public ILayoutSolver {
 public:
  LayoutResult Solve(const IsaDescription& desc) override;

 private:
  struct FieldMeta {
    std::string      name;
    int              min_size;
    bool             flexible;
    std::vector<int> formats;
  };

  IsaDescription                   desc_;
  std::vector<FieldMeta>           fields_;
  std::vector<std::vector<bool>>   free_bits_;
  std::vector<std::pair<int, int>> layout_;

  void InitFields();
  void TraceBits(const FieldMeta& field, int lsb, int msb, bool free);

  [[nodiscard]] bool FindLayout(const size_t field_idx);
  [[nodiscard]] bool CanPlace(const FieldMeta& field, int lsb, int msb) const;
  [[nodiscard]] bool CheckCapacity(size_t field_idx) const;
};
