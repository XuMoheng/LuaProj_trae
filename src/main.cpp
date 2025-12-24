#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Lexer.h"
#include "Parser.h"
#include "AST.h"
#include "Compiler.h"
#include "VM.h"

// Keep AstPrinter for debug flag if needed, but remove from default flow
class AstPrinter : public ExprVisitor, public StmtVisitor {
public:
    void print(const std::vector<std::unique_ptr<Stmt>>& statements) {
        for (const auto& stmt : statements) {
            stmt->accept(this);
        }
    }
    // ... (rest of implementation skipped for brevity as we are replacing it, 
    // actually I should include the implementation or remove the class to avoid linker errors if I called it)
    // For now, I will just comment out the body or implement stubs if not used.
    // To keep it clean, I will remove AstPrinter class from this file 
    // or just not use it.
    
    // Implementation is required if I want to keep it. 
    // Let's remove it to clean up, or keep minimal.
    // I'll remove it to focus on VM execution.
    // If user wants AST dump, they can check previous version or I can re-add.
    // Wait, it's better to keep it but commented out or separate file. 
    // I'll just remove it from main.cpp to make it clean.
    void visitBinaryExpr(BinaryExpr* expr) override {}
    void visitGroupingExpr(GroupingExpr* expr) override {}
    void visitLiteralExpr(LiteralExpr* expr) override {}
    void visitUnaryExpr(UnaryExpr* expr) override {}
    void visitVariableExpr(VariableExpr* expr) override {}
    void visitAssignmentExpr(AssignmentExpr* expr) override {}
    void visitCallExpr(CallExpr* expr) override {}
    void visitExpressionStmt(ExpressionStmt* stmt) override {}
    void visitPrintStmt(PrintStmt* stmt) override {}
    void visitVarDecl(VarDecl* stmt) override {}
    void visitBlockStmt(BlockStmt* stmt) override {}
    void visitIfStmt(IfStmt* stmt) override {}
    void visitWhileStmt(WhileStmt* stmt) override {}
    void visitFunctionStmt(FunctionStmt* stmt) override {}
    void visitReturnStmt(ReturnStmt* stmt) override {}
};

void run(const std::string& source) {
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    Parser parser(tokens);
    try {
        std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
        
        if (statements.empty()) return;

        Chunk chunk;
        Compiler compiler;
        if (compiler.compile(statements, &chunk)) {
            VM vm;
            vm.interpret(&chunk);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void runFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << path << std::endl;
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());
}

void runPrompt() {
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit") break;
        run(line);
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Usage: lua_compiler [script]" << std::endl;
        return 1;
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        runPrompt();
    }
    return 0;
}
