# 抽象语法树 (AST) 与 Visitor 模式详解

AST 是源代码的树状结构表示。它是编译器前端（解析）和后端（解释/代码生成）的接口。

## 1. AST 类层次结构

所有节点都继承自基类。
- `Expr`: 表达式基类 (产生值的节点)
- `Stmt`: 语句基类 (执行动作的节点)

### 示例节点：BinaryExpr (二元表达式)
表示 `a + b` 这样的结构。
```cpp
class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;  // 左操作数
    Token op;                    // 操作符 (+, -, *, /)
    std::unique_ptr<Expr> right; // 右操作数
    // ...
};
```

### 示例节点：IfStmt (If 语句)
表示 `if cond then ... else ... end`。
```cpp
class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition; // 条件
    std::unique_ptr<Stmt> thenBranch; // then 分支
    std::unique_ptr<Stmt> elseBranch; // else 分支 (可选)
    // ...
};
```

## 2. 为什么需要 Visitor 模式？

在 C++ 中，我们要对 AST 执行多种操作，例如：
1.  **Print**: 打印树结构用于调试。
2.  **Interpret**: 执行代码。
3.  **Analyze**: 语义分析（变量定义检查等）。

如果我们将这些逻辑都写在 `Expr` 或 `Stmt` 类的方法里（例如 `virtual void interpret()`），那么每次添加新功能都要修改所有 AST 类的定义。这违反了开闭原则，而且会让 AST 类变得非常臃肿。

**Visitor 模式** 允许我们将操作逻辑与数据结构分离。

### 定义 Visitor 接口
```cpp
class ExprVisitor {
public:
    virtual void visitBinaryExpr(BinaryExpr* expr) = 0;
    virtual void visitLiteralExpr(LiteralExpr* expr) = 0;
    // ... 针对每种 Expr 类型的方法
};
```

### 在 AST 中接受 Visitor
每个 AST 节点只需要实现一个 `accept` 方法：
```cpp
void BinaryExpr::accept(ExprVisitor* visitor) {
    visitor->visitBinaryExpr(this); // 回调 Visitor 的对应方法
}
```

### 实现具体的操作
例如，`AstPrinter` 是一个 Visitor：
```cpp
class AstPrinter : public ExprVisitor {
    void visitBinaryExpr(BinaryExpr* expr) override {
        // 打印逻辑: (op left right)
        std::cout << "(" << expr->op.lexeme << " ";
        expr->left->accept(this);
        std::cout << " ";
        expr->right->accept(this);
        std::cout << ")";
    }
    // ...
};
```

当你调用 `myExpr->accept(&printer)` 时，控制权会通过 `Double Dispatch`（双重分派）机制流转到 `AstPrinter::visitBinaryExpr`，从而执行打印逻辑。

## 3. 内存管理

我们使用 `std::unique_ptr` 来管理 AST 节点的生命周期。
这意味着 AST 是一棵独占所有权的树。当父节点被销毁时，其子节点也会自动被销毁。这大大简化了内存管理，避免了内存泄漏。
