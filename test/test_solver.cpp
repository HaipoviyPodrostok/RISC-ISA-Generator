#include <gtest/gtest.h>

#include "layout_solver.hpp"
#include "types.hpp"

class SolverTest : public ::testing::Test {
 protected:
  IsaDescription GetSimpleDesc() {
    IsaDescription desc;
    desc.total_length = 16;
    desc.fields["R0"] = {"R0", 4, false};
    desc.fields["R1"] = {"R1", 4, false};

    InstrConfig cfg;
    cfg.format   = "ALU";
    cfg.insns    = {"add", "sub"};
    cfg.operands = {"R0", "R1"};
    desc.instructions.push_back(cfg);

    return desc;
  }

  IsaDescription GetFlexibleDesc() {
    IsaDescription desc;
    desc.total_length  = 16;
    desc.fields["R0"]  = {"R0", 4, false};
    desc.fields["imm"] = {"imm", 4, true};

    InstrConfig cfg;
    cfg.format   = "ALU";
    cfg.insns    = {"add"};
    cfg.operands = {"R0", "imm"};
    desc.instructions.push_back(cfg);

    return desc;
  }

  IsaDescription GetImpossibleDesc() {
    IsaDescription desc;
    desc.total_length = 8;
    desc.fields["R0"] = {"R0", 4, false};
    desc.fields["R1"] = {"R1", 4, false};

    InstrConfig cfg;
    cfg.format   = "ALU";
    cfg.insns    = {"add", "sub"};
    cfg.operands = {"R0", "R1"};
    desc.instructions.push_back(cfg);

    return desc;
  }
};

TEST_F(SolverTest, SimpleFixedFields) {
  BacktrackingLayoutSolver solver;
  IsaDescription           desc = GetSimpleDesc();

  LayoutResult res;
  EXPECT_NO_THROW({ res = solver.Solve(desc); });

  EXPECT_EQ(res.size(), 2);
  EXPECT_EQ(res[0].insn, "add");
  EXPECT_EQ(res[1].insn, "sub");
}

TEST_F(SolverTest, FlexibleFieldGreedyExpansion) {
  BacktrackingLayoutSolver solver;
  IsaDescription           desc = GetFlexibleDesc();

  LayoutResult res = solver.Solve(desc);

  EXPECT_EQ(res.size(), 1);
  int imm_size = 0;
  for (const auto& f : res[0].fields) {
    if (f.name == "imm") {
      imm_size = f.msb - f.lsb + 1;
    }
  }

  EXPECT_EQ(imm_size, 12);
}

TEST_F(SolverTest, ImpossibleLayoutThrows) {
  BacktrackingLayoutSolver solver;
  IsaDescription           desc = GetImpossibleDesc();

  EXPECT_THROW(solver.Solve(desc), std::runtime_error);
}
