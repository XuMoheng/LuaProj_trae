# 虚拟机 (VM) 实现详解

虚拟机负责执行字节码。我们的 VM 是一个**栈式虚拟机 (Stack-based VM)**，这意味着指令主要通过操作数栈来交换数据，而不是使用寄存器。

## 1. 核心组件

### 指令指针 (Instruction Pointer - ip)
`ip` 指针始终指向当前正在执行的字节码指令。每次读取指令后，`ip` 自增。

### 操作数栈 (Stack)
用于存储临时值。
*   `stack`: `std::vector<Value>` (实际实现可能使用固定大小数组优化)。
*   `push(val)`: 压栈。
*   `pop()`: 出栈。
*   `peek(distance)`: 查看栈顶下方的元素（不弹出）。

### 全局变量表 (Globals)
`std::unordered_map<std::string, Value>` 用于存储全局变量。

## 2. 解释循环 (Interpret Loop)

VM 的心脏是一个无限循环（`run` 方法），它不断执行“取指-解码-执行”周期：

```cpp
InterpretResult run() {
    for (;;) {
        uint8_t instruction = READ_BYTE(); // 获取当前指令，ip++
        switch (instruction) {
            case OP_ADD: {
                double b = pop(); // 注意顺序：先弹出的是右操作数
                double a = pop();
                push(a + b);
                break;
            }
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            // ... 处理其他指令
        }
    }
}
```

## 3. 关键实现细节

### 二元运算的顺序
在栈式虚拟机中，对于 `a - b`：
1. 先压入 `a`
2. 再压入 `b`
3. 执行 `OP_SUBTRACT` 时，先 `pop()` 出来的是 `b`，再 `pop()` 出来的是 `a`。
所以计算逻辑是 `push(a - b)`。

### 跳转指令
跳转指令（如 `OP_JUMP`）后面跟着两个字节的操作数（16位偏移量）。
执行时，我们将这两个字节组合成一个 `uint16_t`，然后加到 `ip` 上，从而改变执行流。
```cpp
uint16_t offset = READ_SHORT();
ip += offset;
```

### 运行时错误
当操作数类型不正确时（例如对字符串做减法），VM 会调用 `runtimeError` 报告错误并终止执行。错误信息会包含文件名和行号（通过查询 Chunk 的 `lines` 数组）。
