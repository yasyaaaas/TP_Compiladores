# Interpretador Bytecode
Trabalho Prático da disciplina de Compiladores. Implementação de uma máquina virtual de pilha que lê e executa programas escritos em bytecode.<br>
Aluna: Yasmin Casseniro Viegas - 800989<br>
Matéria: Compiladores - PUC Minas<br>

## Como compilar

Na pasta do projeto, rode:

```bash
g++ -std=c++17 -Wall -o interpreter main.cpp interpreter.cpp optimizer.cpp
```

Isso gera o executável `interpreter` (ou `interpreter.exe` no Windows).

## Como rodar

Primeiro copie o executável para a pasta onde estão os arquivos de teste, ou somente arrastar ele para a pasta:

```bash
copy interpreter.exe C:\TP_Compiladores\TESTS-1
```

Depois abra o terminal dentro dessa pasta.

**Exemplo com o in6 dos toytests:**
```bash
./interpreter in6
```

Saída esperada:
```
50
```
Para ver a saída espearda:
```
cat out6
```

## Como o interpretador funciona

O código é dividido em cinco arquivos:

**`main.cpp`** - lê o arquivo de bytecode linha por linha e passa para o interpretador. Também detecta a flag `--optimize` para ativar o otimizador.

**`interpreter.hpp`** - declaração da classe `Interpreter`: lista os atributos e métodos que existem.

**`interpreter.cpp`** - implementação de tudo: o carregamento do bytecode, o loop de execução e cada instrução.

**`optimizer.hpp`** - declaração da classe `Optimizer` com os métodos de otimização.

**`optimizer.cpp`** - implementação das otimizações de bytecode.

### Fase 1 — load()

Antes de executar qualquer coisa, o interpretador lê todas as linhas do bytecode e monta duas estruturas:

- **`program[]`** - lista de instruções na ordem em que aparecem. Cada instrução tem um `opcode` (ex: `PUSH`) e um `operand` (ex: `10`).
- **`labels`** - mapa de nomes para índices. Quando o código tem `LOOP:` ou `LABEL loop`, o interpretador registra que aquele nome aponta para o índice da próxima instrução em `program[]`.<br>
- OBS: Linhas vazias e comentários (tudo após `#`) são ignorados nessa fase.

### Fase 2 — run()

Com o `program[]` montado, o interpretador entra no loop de execução:

1. Lê a instrução no índice `ip` (ponteiro da instrução)
2. Incrementa `ip` para a próxima
3. Identifica o opcode e chama o método correspondente
4. Repete até acabar as instruções ou encontrar `HALT`

Se ocorrer qualquer erro em tempo de execução (divisão por zero, stack overflow, stack underflow), ele é capturado e impresso no formato `# error: mensagem`.

### A pilha

Todas as operações acontecem em uma pilha de inteiros. Por exemplo, para somar 3 + 5:

```
PUSH 3   -> pilha: [3]
PUSH 5   -> pilha: [3, 5]
ADD      -> desempilha os dois, empilha 8 → pilha: [8]
```

A pilha tem limite de 10.000 valores. Se ultrapassar, o programa encerra com `# error: stack overflow`.

### Variáveis

Variáveis são guardadas em um mapa `nome -> valor`. `STORE x` desempilha e salva em `x`. `LOAD x` lê o valor de `x` e empilha.

### Saltos e labels

`JMP`, `JZ` e `JNZ` alteram o `ip` diretamente, fazendo o loop de execução continuar de outro ponto do programa. Os destinos podem ser um nome de label (`JMP LOOP_START`) ou um índice numérico direto (`JZ 5`).

### Funções

`CALL` empilha o endereço de retorno (o `ip` atual) e salta para a função. `RET` desempilha esse endereço e volta para ele, continuando de onde parou.

## Instruções suportadas

| Instrução | O que faz |
|---|---|
| `PUSH n` | Empilha o valor n |
| `POP` | Descarta o topo |
| `ADD` `SUB` `MUL` `DIV` `MOD` | Operações aritméticas entre os dois topos |
| `NEG` | Inverte o sinal do topo |
| `STORE x` | Salva o topo na variável x |
| `LOAD x` | Empilha o valor da variável x |
| `JMP label` | Salta para o label |
| `JZ label` | Salta se o topo for zero |
| `JNZ label` | Salta se o topo for diferente de zero |
| `EQ` `NEQ` `LT` `GT` `LE` `GE` | Comparações: empilha 1 (verdadeiro) ou 0 (falso) |
| `CALL label` | Chama uma função |
| `RET` | Retorna da função |
| `PRINT` | Imprime o topo |
| `READ` | Lê um inteiro da entrada e empilha |
| `HALT` | Para a execução |
| `LABEL nome` | Define um label (não gera instrução) |

## Otimizador de bytecode

O otimizador é ativado com a flag `--optimize`:

```bash
./interpreter --optimize programa
```

Em vez de executar, imprime o bytecode com as instruções desnecessárias removidas. O resultado pode ser executado normalmente:

```bash
./interpreter --optimize programa | ./interpreter
```

Foram implementadas duas otimizações:

### Otimização 1 - Constant folding

Quando duas constantes são empilhadas e logo operadas aritmeticamente, o cálculo é feito em tempo de compilação e as três instruções viram uma só.

```
# antes        # depois
PUSH 3         PUSH 7
PUSH 4    ->
ADD
```

Funciona para `ADD`, `SUB`, `MUL`, `DIV` e `MOD`. Divisão por zero não é otimizada (interpretador trata em runtime).

### Otimização 2 - Eliminação de PUSH/POP

Quando um `PUSH` é imediatamente seguido de um `POP`, o valor é empilhado e descartado sem ser usado. Os dois se cancelam e são removidos.

```
# antes        # depois
PUSH 99        PUSH 1
POP       ->    PRINT
PUSH 1
PRINT
```

As duas otimizações são aplicadas em sequência: constant folding primeiro, depois eliminação de PUSH/POP. Isso permite casos em que o folding cria um par PUSH/POP que a segunda otimização então elimina.

## Testes do otimizador

Os arquivos `opt1` a `opt5` na pasta do projeto são os testes das otimizações:

| Arquivo | Otimização | Antes | Depois |
|---|---|---|---|
| `opt1` | Constant folding (ADD) | 5 linhas | 3 linhas |
| `opt2` | Constant folding (MUL) | 5 linhas | 3 linhas |
| `opt3` | PUSH/POP + folding (ADD) | 7 linhas | 3 linhas |
| `opt4` | Dois pares PUSH/POP | 7 linhas | 3 linhas |
| `opt5` | PUSH/POP antes de STORE | 7 linhas | 5 linhas |

Para rodar um teste:

```bash
./interpreter --optimize opt1  # mostra o bytecode otimizado
./interpreter --optimize opt1 | ./interpreter  # executa o resultado
```
