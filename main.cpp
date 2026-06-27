#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "interpreter.hpp"
#include "optimizer.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> lines;
    bool optimizeMode = false;

    // Detecta a flag --optimize nos argumentos
    std::string filename = "";
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--optimize") {
            optimizeMode = true;
        } else {
            filename = arg;
        }
    }

    if (!filename.empty()) {
        // Lê de arquivo passado como argumento
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Erro: não foi possível abrir o arquivo '" << filename << "'\n";
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

    if (optimizeMode) {
        // Modo otimizador: imprime o bytecode otimizado e encerra
        Optimizer opt;
        std::vector<std::string> optimized = opt.optimize(lines);
        for (const auto& l : optimized) {
            std::cout << l << "\n";
        }
    } else {
        // Modo normal: executa o bytecode
        Interpreter interp;
        interp.load(lines);
        interp.run();
    }

    return 0;
}