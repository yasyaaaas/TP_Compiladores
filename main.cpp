#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "interpreter.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> lines;

    if (argc == 2) {
        // Lê de arquivo passado como argumento
        std::ifstream file(argv[1]);
        if (!file.is_open()) {
            std::cerr << "Erro: não foi possível abrir o arquivo '" << argv[1] << "'\n";
            return 1;
        }
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
    } else {
        // Lê da entrada padrão (stdin)
        std::string line;
        while (std::getline(std::cin, line)) {
            lines.push_back(line);
        }
    }

    Interpreter interp;
    interp.load(lines);
    interp.run();

    return 0;
}