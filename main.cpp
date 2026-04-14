#include <iostream>
#include <string>

#include "json_parser.hpp"

int main(int argc, char** argv) {
  std::string filename = (argc > 1) ? argv[1] : "test/input.json";

  try {
    json_parser::JsonParser parser(filename);
    IsaDescription          desc = parser.Parse();

    std::cout << "Успешно распарсен файл: " << filename << "\n";
    std::cout << "Длина (length): " << desc.total_length << "\n";
    std::cout << "Количество типов полей: " << desc.fields.size() << "\n";
    std::cout << "Количество групп инструкций: " << desc.instructions.size() << "\n";
  }
  catch (const std::exception& e) {
    std::cerr << "Ошибка: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
