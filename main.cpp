#include <fstream>
#include <iostream>
#include <string>

#include "json_parser.hpp"
#include "layout_solver.hpp"
#include "output_formatter.hpp"

int main(int argc, char** argv) {
  std::string filename = (argc > 1) ? argv[1] : "assets/input.json";

  try {
    json_parser::JsonParser parser(filename);
    IsaDescription          desc = parser.Parse();

    BacktrackingLayoutSolver solver;
    LayoutResult             result = solver.Solve(desc);

    JsonOutputFormatter formatter;
    std::ofstream       out("assets/output.json");

    if (!out) {
      std::cerr << "Failed to open assets/output.json\n";
      return 1;
    }

    formatter.Format(result, out);
  }

  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
