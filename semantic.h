// ============================================================
// semantic.h — Semantic Analyzer Header
// Person 4's Module
// Description: Declares the SemanticAnalyzer class that walks
//              the AST and performs semantic checks:
//              - Variable declared before use
//              - No duplicate declarations in same scope
//              - Type compatibility in assignments & expressions
//              - Condition types in if/while
// ============================================================

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <vector>
#include "parser.h"
#include "symbol_table.h"

class SemanticAnalyzer {
public:
    // Constructor: takes a reference to the shared symbol table
    SemanticAnalyzer(SymbolTable& symTable);

    // Main method: walks the AST and performs all semantic checks.
    // Returns true if no errors were found.
    bool analyze(ASTNode* root);

    // Get the list of semantic error messages
    std::vector<std::string> getErrors() const;

private:
    SymbolTable& symbolTable;
    std::vector<std::string> errors;

    void addError(const std::string& msg);

    // Statement analysis
    void analyzeNode(ASTNode* node);
    void analyzeProgram(ProgramNode* node);
    void analyzeDeclaration(DeclarationNode* node);
    void analyzeAssignment(AssignmentNode* node);
    void analyzeIf(IfNode* node);
    void analyzeWhile(WhileNode* node);
    void analyzePrint(PrintNode* node);
    void analyzeInput(InputNode* node);

    // Expression analysis — returns the result type
    // ("int", "float", "string", or "error")
    std::string analyzeExpression(ASTNode* node);
    std::string analyzeBinaryExpr(BinaryExprNode* node);
    std::string analyzeUnaryExpr(UnaryExprNode* node);

    // Type system helpers
    bool areTypesCompatible(const std::string& target, const std::string& source);
    std::string getResultType(const std::string& left, const std::string& right,
                              const std::string& op);
};

#endif // SEMANTIC_H
