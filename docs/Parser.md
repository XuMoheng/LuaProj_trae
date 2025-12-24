# 语法分析器 (Parser) 实现详解

Parser 将 Token 流转换为抽象语法树 (AST)。我们使用 **递归下降 (Recursive Descent)** 算法。

## 1. 上下文无关文法 (CFG)

在编写 Parser 之前，我们需要定义语法规则。
我们使用类似 BNF 的表示法：

```
program     -> declaration* EOF ;
declaration -> funcDecl | varDecl | statement ;
varDecl     -> "local" IDENTIFIER ("=" expression)? ;
statement   -> ifStmt | whileStmt | block | exprStmt ;
block       -> statement* ;
```

每个 `->` 左边的是 **非终结符** (Non-terminal)，右边是它展开的规则。
大写单词（如 `IDENTIFIER`）是 **终结符** (Terminal)，由 Lexer 生成。

## 2. 递归下降实现

递归下降的核心思想是：**每个非终结符对应一个函数**。

### 示例：解析 `while` 语句
规则：`whileStmt -> "while" expression "do" block "end"`

C++ 实现 (`src/Parser.cpp`):
```cpp
std::unique_ptr<Stmt> Parser::whileStatement() {
    // 1. 匹配 "while" (已经在调用此函数前检查过了，或者在这里 consume)
    //    注意：通常调用者会检查 current token 是 WHILE 才会调用此函数
    
    // 2. 解析条件表达式
    std::unique_ptr<Expr> condition = expression();
    
    // 3. 期待 "do"
    consume(TokenType::DO, "Expect 'do' after while condition.");
    
    // 4. 解析循环体 (block)
    std::vector<std::unique_ptr<Stmt>> bodyStmts = block();
    
    // 5. 期待 "end"
    consume(TokenType::END, "Expect 'end' after while loop.");
    
    // 6. 构建 AST 节点
    return std::make_unique<WhileStmt>(std::move(condition), 
           std::make_unique<BlockStmt>(std::move(bodyStmts)));
}
```

## 3. 表达式解析与优先级

表达式解析比较复杂，因为涉及优先级（例如 `*` 高于 `+`）。
我们使用分层的方法处理优先级。

优先级从低到高：
1.  `expression` (赋值)
2.  `or`
3.  `and`
4.  `equality` (`==`, `!=`)
5.  `comparison` (`<`, `<=`, ...)
6.  `term` (`+`, `-`)
7.  `factor` (`*`, `/`)
8.  `unary` (`-`, `not`)
9.  `call` (函数调用)
10. `primary` (数字, 变量,括号表达式)

**实现模式**：
每个优先级级别是一个函数，它调用**更高优先级**的函数，然后处理当前优先级的操作符。

```cpp
// term -> factor ( ( "-" | "+" ) factor )*
std::unique_ptr<Expr> Parser::term() {
    // 先解析左侧 (更高优先级)
    std::unique_ptr<Expr> expr = factor();

    // 只要遇到当前优先级的操作符 (+ 或 -)
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = factor(); // 解析右侧
        // 组合成新的二元表达式
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}
```

这种结构自然地保证了优先级：`factor()` 会先于 `term()` 的加减法部分执行。

## 4. 错误处理

我们实现了 `synchronize()` 方法用于错误恢复。
当遇到语法错误（如缺少分号或括号）时，Parser 会抛出异常，捕获后调用 `synchronize()`。
该方法会丢弃 Token 直到找到一个语句的开始（如 `if`, `local`, `while`），从而允许编译器继续检查后面的代码，而不是遇到第一个错误就停止。
