#include "optimizer.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

// -----------------------------------------
// Helpers internos
// -----------------------------------------

// Remove comentários (#) e espaços/tabs do início e fim da linha
std::string Optimizer::stripLine(const std::string& line) {
    std::string s = line.substr(0, line.find('#'));
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Converte linhas brutas do bytecode em vetor de Instruction (mesmo parse do interpretador)
std::vector<Instruction> Optimizer::parse(const std::vector<std::string>& lines) {
    std::vector<Instruction> result;
    for (const auto& raw : lines) {
        std::string line = stripLine(raw);
        if (line.empty()) continue;

        // Ignora "LABEL nome" e "NOME:" -> labels não são instruções
        {
            std::istringstream probe(line);
            std::string first;
            probe >> first;
            for (auto& c : first) c = (char)toupper(c);
            if (first == "LABEL") continue;
        }
        if (line.back() == ':') continue;

        // Instrução normal
        std::istringstream iss(line);
        Instruction instr;
        iss >> instr.opcode;
        for (auto& c : instr.opcode) c = (char)toupper(c);

        std::string rest;
        if (std::getline(iss >> std::ws, rest)) {
            size_t e = rest.find_last_not_of(" \t\r\n");
            instr.operand = (e == std::string::npos) ? "" : rest.substr(0, e + 1);
        }
        result.push_back(instr);
    }
    return result;
}

// Converte vetor de Instruction de volta para linhas de texto
std::vector<std::string> Optimizer::toLines(const std::vector<Instruction>& instructions) {
    std::vector<std::string> lines;
    for (const auto& instr : instructions) {
        std::string line = instr.opcode;
        if (!instr.operand.empty()) line += " " + instr.operand;
        lines.push_back(line);
    }
    return lines;
}

// -----------------------------------------
// Otimização 1: remoção de PUSH seguido de POP
// -----------------------------------------

// Se tivermos PUSH x imediatamente seguido de POP, os dois se cancelam.
// Exemplo: PUSH 5 / POP -> removidos (2 instruções viram 0)
std::vector<Instruction> Optimizer::removePushPop(const std::vector<Instruction>& in) {
    std::vector<Instruction> out;
    int i = 0;
    while (i < (int)in.size()) {
        // Verifica se a instrução atual é PUSH e a próxima é POP
        if (i + 1 < (int)in.size() &&
            in[i].opcode == "PUSH" &&
            in[i + 1].opcode == "POP") {
            // Descarta os dois e avança duas posições
            i += 2;
        } else {
            out.push_back(in[i]);
            i++;
        }
    }
    return out;
}

// -----------------------------------------
// Otimização 2: constant folding
// -----------------------------------------

// Se tivermos PUSH a / PUSH b / OP (aritmética), calcula o resultado em tempo de
// compilação e substitui as três instruções por um único PUSH com o resultado.
// Exemplo: PUSH 3 / PUSH 4 / ADD -> PUSH 7 (3 instruções viram 1)
// Funciona para: ADD, SUB, MUL, DIV, MOD
std::vector<Instruction> Optimizer::constantFolding(const std::vector<Instruction>& in) {
    std::vector<Instruction> out;
    int i = 0;
    while (i < (int)in.size()) {
        // Verifica padrão PUSH a / PUSH b / OP
        if (i + 2 < (int)in.size() &&
            in[i].opcode == "PUSH" &&
            in[i + 1].opcode == "PUSH") {

            const std::string& op = in[i + 2].opcode;
            bool isArith = (op == "ADD" || op == "SUB" ||
                            op == "MUL" || op == "DIV" || op == "MOD");

            if (isArith) {
                int a = std::stoi(in[i].operand);
                int b = std::stoi(in[i + 1].operand);
                int result = 0;
                bool skip = false;

                if      (op == "ADD") result = a + b;
                else if (op == "SUB") result = a - b;
                else if (op == "MUL") result = a * b;
                else if (op == "DIV") {
                    if (b == 0) { skip = true; } // não otimiza divisão por zero
                    else result = a / b;
                }
                else if (op == "MOD") {
                    if (b == 0) { skip = true; }
                    else result = a % b;
                }

                if (!skip) {
                    // Substitui as 3 instruções por PUSH resultado
                    Instruction folded;
                    folded.opcode  = "PUSH";
                    folded.operand = std::to_string(result);
                    out.push_back(folded);
                    i += 3; // pula PUSH a, PUSH b, OP
                    continue;
                }
            }
        }

        out.push_back(in[i]);
        i++;
    }
    return out;
}

// -----------------------------------------
// Ponto de entrada da otimização
// -----------------------------------------

// Aplica as otimizações em sequência e devolve as linhas otimizadas
std::vector<std::string> Optimizer::optimize(const std::vector<std::string>& lines) {
    std::vector<Instruction> instrs = parse(lines);

    // Aplica constant folding primeiro (reduz sequências aritméticas)
    instrs = constantFolding(instrs);

    // Depois remove PUSH/POP que possam ter sobrado ou já existiam
    instrs = removePushPop(instrs);

    return toLines(instrs);
}