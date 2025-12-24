# 编译器后端 (Compiler) 实现详解

Compiler 类（`src/Compiler.cpp`）负责将 AST 转换为字节码。它也使用了 Visitor 模式遍历 AST。

## 1. 代码生成策略

编译器的任务是将高层的语法结构翻译成底层的栈操作指令。

### 表达式编译
表达式编译的结果是将计算出的值留在栈顶。

*   **字面量 (`123`)**: 发射 `OP_CONSTANT` 指令。
*   **二元运算 (`1 + 2`)**:
    1.  递归编译左子树 (栈顶: `1`)
    2.  递归编译右子树 (栈顶: `1`, `2`)
    3.  发射 `OP_ADD` (栈顶: `3`)

### 语句编译
语句通常涉及副作用或控制流，执行完后通常不留值在栈上（或者会清理）。

*   **表达式语句**: 编译表达式，然后发射 `OP_POP`（丢弃结果）。
*   **变量声明 (`local a = 1`)**:
    1.  编译初始化表达式 (栈顶: `1`)
    2.  发射 `OP_DEFINE_GLOBAL` (将栈顶值存入全局表)

### 控制流编译 (回填技术)
编译 `if` 语句时，我们还不知道要跳转多远（因为还没编译 `else` 块）。我们使用**回填 (Backpatching)** 技术。

**流程 (`if condition then ... else ... end`)**:
1.  编译 `condition`。
2.  发射 `OP_JUMP_IF_FALSE`，但在偏移量位置写入占位符 (0xFFFF)，并记录下这个位置 (`thenJump`)。
3.  发射 `OP_POP` (清理栈上的 condition)。
4.  编译 `then` 分支。
5.  发射 `OP_JUMP` (跳过 else 分支)，同样记录位置 (`elseJump`)。
6.  **回填 `thenJump`**: 计算从步骤 2 到现在的字节数，更新 `thenJump` 处的偏移量。
7.  发射 `OP_POP` (清理栈上的 condition，针对 false 的情况)。
8.  编译 `else` 分支。
9.  **回填 `elseJump`**: 更新偏移量。

## 2. 关键函数

*   `emitByte(byte)`: 写入一个字节到当前 Chunk。
*   `emitConstant(value)`: 添加常量并写入 `OP_CONSTANT` 指令。
*   `emitJump(instruction)`: 写入跳转指令和占位符，返回占位符的索引。
*   `patchJump(offset)`: 计算当前位置与 `offset` 的距离，并重写该处的跳转偏移量。

## 3. 示例：While 循环

`while a < 10 do ... end`

1.  记录循环开始位置 `loopStart`。
2.  编译 `a < 10`。
3.  发射 `OP_JUMP_IF_FALSE` (跳出循环)，记录 `exitJump`。
4.  发射 `OP_POP`。
5.  编译循环体。
6.  发射 `OP_LOOP`，偏移量指向 `loopStart` (回跳)。
7.  回填 `exitJump`。
8.  发射 `OP_POP`。
