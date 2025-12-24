# 从零开始构建 Lua 编译器：实现指南

欢迎来到 Lua 编译器构建教程。本指南将带你一步步了解如何使用 C++ 从零开始构建一个简单的 Lua 编译器。

## 1. 编译器基础：它是如何工作的？

编译器本质上是一个将源代码（人类可读的文本）转换为目标代码（机器或虚拟机可执行的代码）的程序。
我们的编译器包含以下几个核心阶段：

1.  **词法分析 (Lexical Analysis / Scanning)**
    *   **输入**: 源代码字符串 (例如 `local a = 10`)
    *   **输出**: Token 流 (例如 `LOCAL`, `IDENTIFIER("a")`, `EQUAL`, `NUMBER(10)`)
    *   **任务**: 将字符组合成有意义的单词（Token）。

2.  **语法分析 (Syntax Analysis / Parsing)**
    *   **输入**: Token 流
    *   **输出**: 抽象语法树 (AST)
    *   **任务**: 验证 Token 的排列是否符合语法规则，并构建出树状结构来表示代码逻辑。

3.  **代码生成 (Code Generation)**
    *   **输入**: AST
    *   **输出**: 字节码 Chunk
    *   **任务**: 将树状的 AST 拍平为线性的指令序列。

4.  **虚拟机执行 (Virtual Machine)**
    *   **输入**: 字节码 Chunk
    *   **输出**: 程序运行结果
    *   **任务**: 模拟 CPU 行为，使用栈来计算和管理状态。

---

## 2. 环境搭建

我们需要 C++ 编译器和 CMake 构建系统。

**项目结构**:
```
lua_compiler/
├── CMakeLists.txt      # 构建脚本
├── include/            # 头文件
│   ├── Token.h         # Token 定义
│   ├── Lexer.h         # 词法分析器
│   ├── AST.h           # 语法树节点
│   ├── Parser.h        # 语法分析器
│   ├── Chunk.h         # 字节码结构
│   ├── Compiler.h      # 编译器
│   └── VM.h            # 虚拟机
├── src/                # 源代码
│   ├── Lexer.cpp
│   ├── Parser.cpp
│   ├── Compiler.cpp
│   ├── VM.cpp
│   └── main.cpp        # 入口
```

在 `CMakeLists.txt` 中，我们需要定义项目并包含源文件。

---

## 3. 前端实现：Lexer, AST, Parser

前端部分负责理解源代码。这部分在之前的步骤中已经详细讲解。

*   **Lexer**: 使用状态机模式扫描字符。详见 [Lexer 文档](Lexer.md)。
*   **AST**: 使用 Visitor 模式定义的树结构。详见 [AST 文档](AST.md)。
*   **Parser**: 使用递归下降算法解析语法。详见 [Parser 文档](Parser.md)。

---

## 4. 后端实现：Compiler 与 VM

这是编译器真正的“引擎”部分。我们不再直接解释 AST（那很慢），而是将其编译成字节码，然后在高效的虚拟机上运行。

### 4.1 字节码 (Bytecode)
字节码是紧凑的指令序列。
例如，`1 + 2` 会被编译成：
```
OP_CONSTANT 1
OP_CONSTANT 2
OP_ADD
```
详见 [Bytecode 文档](Bytecode.md)。

### 4.2 编译器 (Compiler)
编译器是一个 Visitor，它遍历 AST 并“发射”对应的指令。
例如，当它访问 `BinaryExpr` 时：
1.  递归访问左子节点（这会将左操作数压栈）。
2.  递归访问右子节点（这会将右操作数压栈）。
3.  根据操作符，发射 `OP_ADD` / `OP_SUB` 等指令。

详见 [Compiler 文档](Compiler.md)。

### 4.3 虚拟机 (VM)
VM 维护一个**栈 (Stack)** 和一个**指令指针 (ip)**。
它在一个无限循环中读取 `ip` 指向的指令，执行它（通常涉及压栈/弹栈），然后移动 `ip`。

详见 [VM 文档](VM.md)。

---

## 5. 驱动程序 (Main)

`main.cpp` 将所有部分串联起来：

```cpp
// 1. 扫描
Lexer lexer(source);
vector<Token> tokens = lexer.scanTokens();

// 2. 解析
Parser parser(tokens);
vector<unique_ptr<Stmt>> statements = parser.parse();

// 3. 编译
Chunk chunk;
Compiler compiler;
compiler.compile(statements, &chunk);

// 4. 执行
VM vm;
vm.interpret(&chunk);
```

---

## 6. 总结

通过这个项目，你已经实现了一个具备完整功能的微型语言编译器！
它涵盖了编译器原理的所有核心领域：词法分析、语法分析、语义分析（通过编译过程体现）、中间代码生成和虚拟机运行时。
