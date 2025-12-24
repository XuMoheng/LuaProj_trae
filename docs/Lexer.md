# 词法分析器 (Lexer) 实现详解

Lexer 的任务是将源代码文本流转换为 Token 流。它是编译器的第一道工序。

## 1. Token 的定义

在 `include/Token.h` 中，我们定义了 `Token` 结构体和 `TokenType` 枚举。

```cpp
enum class TokenType {
    // 单字符 Token
    LEFT_PAREN, RIGHT_PAREN, ..., 
    // 双字符 Token
    EQUAL_EQUAL, LESS_EQUAL, ...,
    // 字面量
    IDENTIFIER, STRING, NUMBER,
    // 关键字
    LOCAL, FUNCTION, IF, ...
};
```

`Token` 结构体包含：
- `type`: Token 的类型。
- `lexeme`: Token 在源码中的原始字符串（例如变量名 "myVar"）。
- `line/column`: 用于错误报告的位置信息。

## 2. 状态机与扫描循环

Lexer 的工作方式类似于一个有限状态机（FSM）。
核心函数 `scanToken()` 每次调用识别一个 Token。

### 扫描过程
1.  记录 `start` 位置。
2.  读取下一个字符 `c`。
3.  根据 `c` 决定下一步：
    - 如果是 `(`，直接生成 `LEFT_PAREN` Token。
    - 如果是 `=`，查看下一个字符：
        - 如果下一个是 `=`，生成 `EQUAL_EQUAL` (`==`)。
        - 否则，生成 `EQUAL` (`=`)。
    - 如果是数字，进入 `number()` 处理函数，不断读取数字直到非数字字符。
    - 如果是字母，进入 `identifier()` 处理函数，读取直到非字母数字字符，然后查表判断是否是关键字。

## 3. 关键实现细节

### 处理注释
Lua 的注释以 `--` 开头。
```cpp
case '-':
    if (match('-')) {
        // 是注释，一直读取直到换行
        while (peek() != '\n' && !isAtEnd()) advance();
    } else {
        addToken(TokenType::MINUS);
    }
    break;
```

### 关键字 vs 标识符
当我们扫描到一个单词（如 `local`）时，它既符合标识符的规则，也可能是一个关键字。
我们需要一个哈希表 (`std::unordered_map`) 来存储所有关键字。扫描完单词后，在表中查找：
- 存在 -> 它是关键字 (`TokenType::LOCAL`)。
- 不存在 -> 它是普通标识符 (`TokenType::IDENTIFIER`)。

### 字符串处理
当遇到 `"` 时，进入字符串模式。
不断读取字符直到遇到另一个 `"`。
注意处理换行符（允许跨行字符串）和文件结束（未闭合字符串错误）。

## 4. 代码导读
请对照 `src/Lexer.cpp` 阅读：
- `advance()`: 消耗并返回下一个字符。
- `peek()`: 查看当前字符但不消耗。
- `match(expected)`: 如果当前字符匹配则消耗并返回 true，否则返回 false。
