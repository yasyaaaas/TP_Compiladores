#include "interpreter.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Limite de tamanho da pilha para detectar stack overflow
static const int STACK_LIMIT = 10000;

// -----------------------------------------
// Helpers internos
// -----------------------------------------

// Remove comentários (#) e espaço/tab do início e fim da linha
std::string Interpreter::stripLine(const std::string& line) {
    std::string s = line.substr(0, line.find('#'));
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Desempilha e retorna o topo, lança erro se a pilha estiver vazia (stack underflow)
int Interpreter::pop() {
    if (stack.empty())
        throw std::runtime_error("stack underflow");
    int v = stack.top();
    stack.pop();
    return v;
}

// Empilha um valor, lança erro se a pilha atingir o limite (stack overflow)
void Interpreter::push(int value) {
    if ((int)stack.size() >= STACK_LIMIT)
        throw std::runtime_error("stack overflow");
    stack.push(value);
}

// Converte um target (label ou número) para índice de instrução no program[]
int Interpreter::resolveTarget(const std::string& target) {
    try {
        return std::stoi(target);
    } catch (...) {}
    auto it = labels.find(target);
    if (it == labels.end())
        throw std::runtime_error("label nao encontrado: " + target);
    return it->second;
}

// -----------------------------------------
// Carregamento e resolução de labels
// -----------------------------------------

// Lê todas as linhas do bytecode, registra labels e monta o vetor program[]
void Interpreter::load(const std::vector<std::string>& lines) {
    for (const auto& raw : lines) {
        std::string line = stripLine(raw);
        if (line.empty()) continue;

        // Formato "LABEL nome" ->  instrução explícita de label (ignorada em runtime,
        // mas o label aponta para a próxima instrução real)
        {
            std::istringstream probe(line);
            std::string first;
            probe >> first;
            for (auto& c : first) c = (char)toupper(c);
            if (first == "LABEL") {
                std::string labelName;
                if (probe >> labelName) {
                    labels[labelName] = (int)program.size();
                }
                continue; // não vira instrução
            }
        }

        // Formato "NOME:" ->  label inline sozinho na linha
        if (line.back() == ':') {
            std::string label = line.substr(0, line.size() - 1);
            size_t e = label.find_last_not_of(" \t");
            if (e != std::string::npos) label = label.substr(0, e + 1);
            labels[label] = (int)program.size();
            continue;
        }

        // Instrução normal: lê opcode e operando e adiciona em program[]
        std::istringstream iss(line);
        Instruction instr;
        iss >> instr.opcode;
        for (auto& c : instr.opcode) c = (char)toupper(c);

        std::string rest;
        if (std::getline(iss >> std::ws, rest)) {
            size_t e = rest.find_last_not_of(" \t\r\n");
            instr.operand = (e == std::string::npos) ? "" : rest.substr(0, e + 1);
        }

        program.push_back(instr);
    }
}

// -----------------------------------------
// Loop principal de execução
// -----------------------------------------

// Executa o programa a partir do ip=0, despachando cada opcode para seu método
void Interpreter::run() {
    ip = 0;
    try {
        while (ip < (int)program.size()) {
            const Instruction instr = program[ip];
            ip++; // incrementa antes de executar para que saltos sobrescrevam ip corretamente

            const std::string& op  = instr.opcode;
            const std::string& arg = instr.operand;

            if      (op == "PUSH")  execPUSH(arg);
            else if (op == "POP")   execPOP();
            else if (op == "ADD")   execADD();
            else if (op == "SUB")   execSUB();
            else if (op == "MUL")   execMUL();
            else if (op == "DIV")   execDIV();
            else if (op == "MOD")   execMOD();
            else if (op == "NEG")   execNEG();
            else if (op == "STORE") execSTORE(arg);
            else if (op == "LOAD")  execLOAD(arg);
            else if (op == "JMP")   execJMP(arg);
            else if (op == "JZ")    execJZ(arg);
            else if (op == "JNZ")   execJNZ(arg);
            else if (op == "EQ")    execEQ();
            else if (op == "NEQ")   execNEQ();
            else if (op == "LT")    execLT();
            else if (op == "GT")    execGT();
            else if (op == "LE")    execLE();
            else if (op == "GE")    execGE();
            else if (op == "CALL")  execCALL(arg);
            else if (op == "RET")   execRET();
            else if (op == "PRINT") execPRINT();
            else if (op == "READ")  execREAD();
            else if (op == "HALT")  return; // para a execução imediatamente
            else {
                throw std::runtime_error("opcode desconhecido: " + op);
            }
        }
    } catch (const std::runtime_error& e) {
        // Erros de runtime vão pro stdout
        std::cout << "# error: " << e.what() << "\n";
    }
}

// -----------------------------------------
// Aritmética e pilha
// -----------------------------------------

// Converte o operando para inteiro e empilha
void Interpreter::execPUSH(const std::string& operand) {
    push(std::stoi(operand));
}

// Descarta o topo da pilha
void Interpreter::execPOP() {
    pop();
}

// Desempilha dois valores e empilha a soma (a + b)
void Interpreter::execADD() {
    int b = pop(), a = pop();
    push(a + b);
}

// Desempilha dois valores e empilha a subtração (a - b)
void Interpreter::execSUB() {
    int b = pop(), a = pop();
    push(a - b);
}

// Desempilha dois valores e empilha o produto (a * b)
void Interpreter::execMUL() {
    int b = pop(), a = pop();
    push(a * b);
}

// Desempilha dois valores e empilha a divisão inteira (a / b); erro se b == 0
void Interpreter::execDIV() {
    int b = pop(), a = pop();
    if (b == 0) throw std::runtime_error("div by zero");
    push(a / b);
}

// Desempilha dois valores e empilha o resto (a % b); erro se b == 0
void Interpreter::execMOD() {
    int b = pop(), a = pop();
    if (b == 0) throw std::runtime_error("mod by zero");
    push(a % b);
}

// Inverte o sinal do topo da pilha
void Interpreter::execNEG() {
    push(-pop());
}

// -----------------------------------------
// Variáveis
// -----------------------------------------

// Desempilha o topo e salva na memória com o nome da variável
void Interpreter::execSTORE(const std::string& var) {
    mem[var] = pop();
}

// Busca a variável na memória e empilha seu valor
void Interpreter::execLOAD(const std::string& var) {
    auto it = mem.find(var);
    if (it == mem.end())
        throw std::runtime_error("variavel nao definida: " + var);
    push(it->second);
}

// -----------------------------------------
// Fluxo de controle
// -----------------------------------------

// Salta incondicionalmente para o endereço/label indicado
void Interpreter::execJMP(const std::string& target) {
    ip = resolveTarget(target);
}

// Desempilha e salta para o target somente se o valor for zero
void Interpreter::execJZ(const std::string& target) {
    int v = pop();
    if (v == 0) ip = resolveTarget(target);
}

// Desempilha e salta para o target somente se o valor for diferente de zero
void Interpreter::execJNZ(const std::string& target) {
    int v = pop();
    if (v != 0) ip = resolveTarget(target);
}

// -----------------------------------------
// Comparação
// -----------------------------------------

// Empilha 1 se a == b, 0 caso contrário
void Interpreter::execEQ()  { int b = pop(), a = pop(); push(a == b ? 1 : 0); }

// Empilha 1 se a != b, 0 caso contrário
void Interpreter::execNEQ() { int b = pop(), a = pop(); push(a != b ? 1 : 0); }

// Empilha 1 se a < b, 0 caso contrário
void Interpreter::execLT()  { int b = pop(), a = pop(); push(a <  b ? 1 : 0); }

// Empilha 1 se a > b, 0 caso contrário
void Interpreter::execGT()  { int b = pop(), a = pop(); push(a >  b ? 1 : 0); }

// Empilha 1 se a <= b, 0 caso contrário
void Interpreter::execLE()  { int b = pop(), a = pop(); push(a <= b ? 1 : 0); }

// Empilha 1 se a >= b, 0 caso contrário
void Interpreter::execGE()  { int b = pop(), a = pop(); push(a >= b ? 1 : 0); }

// -----------------------------------------
// Funções e E/S
// -----------------------------------------

// Empilha o endereço de retorno (ip atual) e salta para a função
void Interpreter::execCALL(const std::string& target) {
    push(ip);  // ip já aponta para a instrução após o CALL
    ip = resolveTarget(target);
}

// Desempilha o endereço de retorno e volta para ele
void Interpreter::execRET() {
    ip = pop();
}

// Imprime o topo da pilha e o descarta; silencioso se pilha vazia
void Interpreter::execPRINT() {
    if (stack.empty()) return;
    std::cout << pop() << "\n";
}

// Lê um inteiro da entrada padrão e empilha
void Interpreter::execREAD() {
    int v;
    if (!(std::cin >> v))
        throw std::runtime_error("erro de leitura");
    push(v);
}