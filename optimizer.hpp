#pragma once

#include "interpreter.hpp"
#include <vector>
#include <string>

class Optimizer {
public:
    // Recebe as linhas do bytecode e devolve elas otimizadas
    std::vector<std::string> optimize(const std::vector<std::string>& lines);

private:
    // Converte linhas originais em lista de Instruction
    std::vector<Instruction> parse(const std::vector<std::string>& lines);

    // Remove comentários e espaços extras de uma linha
    std::string stripLine(const std::string& line);

    // Otimização 1: PUSH x seguido de POP -> remove os dois (não fazem nada)
    std::vector<Instruction> removePushPop(const std::vector<Instruction>& instructions);

    // Otimização 2: PUSH a, PUSH b, OP -> PUSH (resultado) -. constant folding
    std::vector<Instruction> constantFolding(const std::vector<Instruction>& instructions);

    // Converte a lista de Instruction de volta para linhas de texto
    std::vector<std::string> toLines(const std::vector<Instruction>& instructions);
};