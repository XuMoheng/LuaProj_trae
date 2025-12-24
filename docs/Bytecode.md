# 字节码与指令集 (Bytecode & OpCodes)

字节码是编译器生成的中间表示，它是一种专为虚拟机设计的紧凑指令集。相比于直接解释 AST，字节码执行效率更高，且更容易序列化。

## 1. Chunk (代码块)

`Chunk` 是字节码的容器。它包含：
*   **指令序列 (`code`)**: 一个 `uint8_t` 数组，存储操作码 (OpCode) 和操作数。
*   **常量池 (`constants`)**: 存储代码中用到的字面量（数字、字符串），指令中通过索引引用这些常量。
*   **行号信息 (`lines`)**: 记录每个字节码对应的源码行号，用于报错。

## 2. 指令集 (OpCodes)

我们定义了一组基于栈的指令集。

### 栈操作
*   `OP_CONSTANT (idx)`: 将常量池中索引为 `idx` 的值压入栈顶。
*   `OP_POP`: 弹出栈顶元素。
*   `OP_NIL`, `OP_TRUE`, `OP_FALSE`: 将对应值压入栈顶。

### 变量操作
*   `OP_DEFINE_GLOBAL (idx)`: 定义全局变量。变量名在常量池 `idx` 处。取栈顶值作为初始值。
*   `OP_GET_GLOBAL (idx)`: 获取全局变量的值并压入栈。
*   `OP_SET_GLOBAL (idx)`: 设置全局变量的值（使用栈顶值，但不弹出，以便连等赋值）。

### 算术与逻辑
所有二元操作都从栈弹出两个值，计算后将结果压入栈。
*   `OP_ADD` (+), `OP_SUBTRACT` (-), `OP_MULTIPLY` (*), `OP_DIVIDE` (/)
*   `OP_NEGATE` (-): 取反栈顶数值。
*   `OP_NOT` (not): 逻辑取反。
*   `OP_EQUAL` (==), `OP_GREATER` (>), `OP_LESS` (<)

### 控制流
*   `OP_JUMP (offset)`: 无条件跳转。`offset` 是 16 位整数，表示向前跳过的字节数。
*   `OP_JUMP_IF_FALSE (offset)`: 如果栈顶为假（false 或 nil），则跳转；否则继续执行。
*   `OP_LOOP (offset)`: 向后跳转（回跳），用于实现循环。

### 其他
*   `OP_PRINT`: 弹出栈顶值并打印（用于调试或 `print` 函数）。
*   `OP_RETURN`: 停止执行。

## 3. 指令编码示例

源码：
```lua
print(1 + 2)
```

编译后的字节码序列 (逻辑视图):
```
OP_CONSTANT 0 (value: 1)
OP_CONSTANT 1 (value: 2)
OP_ADD
OP_PRINT
OP_NIL      (print 函数返回值)
OP_POP      (弹出返回值)
OP_RETURN
```
