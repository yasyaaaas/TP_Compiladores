#pragma once

#include <string>
#include <vector>
#include <stack>
#include <unordered_map>

// Representa uma instrução já parseada
struct Instruction {
    std::string opcode;
    std::string operand; // pode ser vazio, um número ou uma label
};

class Interpreter {
public:
    // Carrega as linhas do bytecode, faz o pre-processamento de labels
    void load(const std::vector<std::string>& lines);

    // Executa o programa a partir da instrução 0
    void run();

private:
    // Estado da VM
    std::vector<Instruction> program;            // instruções carregadas
    std::stack<int> stack;                       // pilha de valores inteiros
    std::unordered_map<std::string, int> mem;    // memória de variáveis (nome -> valor)
    std::unordered_map<std::string, int> labels; // label -> índice na lista de instruções
    int ip = 0;                                  // ponteiro da intrução

    // Helpers de parse
    // Remove comentários e espaços extras de uma linha
    std::string stripLine(const std::string& line);

    // Faz o primeiro passo: coleta todos os labels e seus endereços
    void resolveLabels();

    // Execução de cada instrução
    void execPUSH(const std::string& operand);
    void execPOP();
    void execADD();
    void execSUB();
    void execMUL();
    void execDIV();
    void execMOD();
    void execNEG();

    void execSTORE(const std::string& var);
    void execLOAD(const std::string& var);

    void execJMP(const std::string& target);
    void execJZ(const std::string& target);
    void execJNZ(const std::string& target);
    // HALT é tratado em run()

    void execEQ();
    void execNEQ();
    void execLT();
    void execGT();
    void execLE();
    void execGE();

    void execCALL(const std::string& target);
    void execRET();
    void execPRINT();
    void execREAD();

    // Utilitários
    int pop(); // desempilha e retorna; lança erro se vazia
    void push(int value);

    // Resolve target: se for número usa direto, se for label busca no mapa
    int resolveTarget(const std::string& target);
};