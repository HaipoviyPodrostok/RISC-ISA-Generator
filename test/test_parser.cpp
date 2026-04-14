#include <gtest/gtest.h>

#include <fstream>
#include <stdexcept>
#include <string>

#include "json_parser.hpp"

class ParserTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::ofstream out("test_valid.json");
    out << R"({
      "length": "25",
      "fields": [
        {"R0": "5"},
        {"imm": ">=8"}
      ],
      "instructions": [
        {
          "format": "F1",
          "insns": ["add", "sub"],
          "operands": ["R0", "imm"]
        }
      ]
    })";
    out.close();

    std::ofstream err("test_invalid.json");
    err << R"({ "length": "25" })";
    err.close();

    std::ofstream miss("test_missing_op.json");
    miss << R"({
      "length": "25",
      "fields": [ {"R0": "5"} ],
      "instructions": [
        {
          "format": "F1",
          "insns": ["add"],
          "operands": ["R0", "R1"]
        }
      ]
    })";
    miss.close();
  }

  void TearDown() override {
    std::remove("test_valid.json");
    std::remove("test_invalid.json");
    std::remove("test_missing_op.json");
  }
};

TEST_F(ParserTest, ParseValidJson) {
  json_parser::JsonParser parser("test_valid.json");
  IsaDescription          desc = parser.Parse();

  EXPECT_EQ(desc.total_length, 25);
  EXPECT_EQ(desc.fields.size(), 2);
  EXPECT_TRUE(desc.fields.at("imm").is_flexible);
  EXPECT_EQ(desc.fields.at("imm").min_size, 8);
  EXPECT_EQ(desc.instructions.size(), 1);
  EXPECT_EQ(desc.instructions[0].insns.size(), 2);
}

TEST_F(ParserTest, ParseMissingArrays) {
  json_parser::JsonParser parser("test_invalid.json");
  EXPECT_THROW(parser.Parse(), std::runtime_error);
}

TEST_F(ParserTest, UndefinedOperandThrows) {
  json_parser::JsonParser parser("test_missing_op.json");
  EXPECT_THROW(parser.Parse(), std::runtime_error);
}
