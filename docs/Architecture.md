# Lua Compiler Architecture

## Overview
This project builds a basic Lua compiler and runtime environment from scratch using C++.
The pipeline consists of four main stages:

1.  **Lexer (Lexical Analysis)**: Source Code -> Tokens
2.  **Parser (Syntax Analysis)**: Tokens -> AST (Abstract Syntax Tree)
3.  **Compiler (Code Generation)**: AST -> Bytecode (Chunk)
4.  **VM (Virtual Machine)**: Bytecode -> Execution Result

## Directory Structure
- `src/`: Source files (.cpp)
- `include/`: Header files (.h)
- `docs/`: Documentation
    - [Tutorial.md](Tutorial.md): From scratch implementation guide.
    - [Lexer.md](Lexer.md): Tokenization details.
    - [Parser.md](Parser.md): Recursive descent parsing and grammar.
    - [AST.md](AST.md): AST structure and Visitor pattern.
    - [Bytecode.md](Bytecode.md): Instruction set architecture.
    - [Compiler.md](Compiler.md): AST to bytecode compilation.
    - [VM.md](VM.md): Stack-based virtual machine internals.
- `tests/`: Test scripts

## Core Components

### Frontend (Analysis)
*   **Lexer**: Converts raw text into a stream of tokens (keywords, identifiers, symbols).
*   **Parser**: Verifies syntax and builds a hierarchical tree structure (AST) representing the code.
*   **AST**: Uses the Visitor pattern to decouple data structure from operations (printing, compiling).

### Backend (Synthesis & Execution)
*   **Compiler**: Traverses the AST and emits linear bytecode instructions. Handles control flow via jump patching.
*   **Chunk**: A container for bytecode instructions and constants.
*   **VM**: A stack-based interpreter that executes the bytecode. It manages the runtime stack, global variables, and instruction dispatch.
