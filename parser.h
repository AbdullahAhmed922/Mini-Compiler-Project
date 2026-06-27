// ============================================================
// parser.h — Parser (Syntax Analyzer) Header
// Person 3's Module
// Description: Declares AST node types and the Parser class.
//              The parser takes a vector of tokens and builds
//              an Abstract Syntax Tree (AST) using recursive
//              descent parsing.
// ============================================================

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "token.h"

// ==================== AST Node Types ====================

enum class NodeType {
    PROGRAM,
    DECLARATION,
    ASSIGNMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    PRINT_STATEMENT,
    INPUT_STATEMENT,
    BINARY_EXPR,
    UNARY_EXPR,
    LITERAL,
    IDENTIFIER
};

// ==================== Base AST Node ====================
struct ASTNode {
    NodeType nodeType;
    int line;   // source line number for error reporting

    ASTNode(NodeType type, int l) : nodeType(type), line(l) {}
    virtual ~ASTNode() {}
    virtual void print(int indent = 0) const = 0;
};

// ==================== Statement Nodes ====================

// ProgramNode — root of the AST, contains a list of statements
struct ProgramNode : public ASTNode {
    std::vector<ASTNode*> statements;

    ProgramNode() : ASTNode(NodeType::PROGRAM, 0) {}
    ~ProgramNode();
    void print(int indent = 0) const override;
};

// DeclarationNode — e.g., "int x;"
struct DeclarationNode : public ASTNode {
    std::string dataType;   // "int", "float", "string"
    std::string varName;    // variable name

    DeclarationNode(const std::string& type, const std::string& name, int l)
        : ASTNode(NodeType::DECLARATION, l), dataType(type), varName(name) {}
    void print(int indent = 0) const override;
};

// AssignmentNode — e.g., "x = expr;"
struct AssignmentNode : public ASTNode {
    std::string varName;
    ASTNode* expression;

    AssignmentNode(const std::string& name, ASTNode* expr, int l)
        : ASTNode(NodeType::ASSIGNMENT, l), varName(name), expression(expr) {}
    ~AssignmentNode();
    void print(int indent = 0) const override;
};

// IfNode — if (condition) { ... } else { ... }
struct IfNode : public ASTNode {
    ASTNode* condition;
    std::vector<ASTNode*> thenBody;
    std::vector<ASTNode*> elseBody;     // empty if no else

    IfNode(ASTNode* cond, int l)
        : ASTNode(NodeType::IF_STATEMENT, l), condition(cond) {}
    ~IfNode();
    void print(int indent = 0) const override;
};

// WhileNode — while (condition) { ... }
struct WhileNode : public ASTNode {
    ASTNode* condition;
    std::vector<ASTNode*> body;

    WhileNode(ASTNode* cond, int l)
        : ASTNode(NodeType::WHILE_STATEMENT, l), condition(cond) {}
    ~WhileNode();
    void print(int indent = 0) const override;
};

// PrintNode — print(expr);
struct PrintNode : public ASTNode {
    ASTNode* expression;

    PrintNode(ASTNode* expr, int l)
        : ASTNode(NodeType::PRINT_STATEMENT, l), expression(expr) {}
    ~PrintNode();
    void print(int indent = 0) const override;
};

// InputNode — input(varName);
struct InputNode : public ASTNode {
    std::string varName;

    InputNode(const std::string& name, int l)
        : ASTNode(NodeType::INPUT_STATEMENT, l), varName(name) {}
    void print(int indent = 0) const override;
};

// ==================== Expression Nodes ====================

// BinaryExprNode — left op right (e.g., x + y, a > b)
struct BinaryExprNode : public ASTNode {
    std::string op;         // "+", "-", "*", "/", "==", etc.
    ASTNode* left;
    ASTNode* right;

    BinaryExprNode(const std::string& oper, ASTNode* l, ASTNode* r, int ln)
        : ASTNode(NodeType::BINARY_EXPR, ln), op(oper), left(l), right(r) {}
    ~BinaryExprNode();
    void print(int indent = 0) const override;
};

// UnaryExprNode — op operand (e.g., -x, !flag)
struct UnaryExprNode : public ASTNode {
    std::string op;         // "-" or "!"
    ASTNode* operand;

    UnaryExprNode(const std::string& oper, ASTNode* opnd, int l)
        : ASTNode(NodeType::UNARY_EXPR, l), op(oper), operand(opnd) {}
    ~UnaryExprNode();
    void print(int indent = 0) const override;
};

// LiteralNode — a constant value (10, 3.14, "hello")
struct LiteralNode : public ASTNode {
    std::string value;
    TokenType literalType;  // INTEGER_LITERAL, FLOAT_LITERAL, STRING_LITERAL

    LiteralNode(const std::string& val, TokenType type, int l)
        : ASTNode(NodeType::LITERAL, l), value(val), literalType(type) {}
    void print(int indent = 0) const override;
};

// IdentifierNode — a variable reference (x, sum, name)
struct IdentifierNode : public ASTNode {
    std::string name;

    IdentifierNode(const std::string& n, int l)
        : ASTNode(NodeType::IDENTIFIER, l), name(n) {}
    void print(int indent = 0) const override;
};


// ==================== Parser Class ====================

class Parser {
public:
    // Constructor: takes the token list from the Lexer
    Parser(const std::vector<Token>& tokens);

    // Main method: parses tokens and returns the AST root
    ASTNode* parse();

    // Error checking
    bool hasErrors() const;
    std::vector<std::string> getErrors() const;

private:
    std::vector<Token> tokens;
    int pos;                        // current position in token list
    std::vector<std::string> errors;

    // Token access helpers
    Token currentToken();
    Token peekToken();
    void advance();
    bool match(TokenType type);
    bool expect(TokenType type, const std::string& errMsg);
    void addError(const std::string& msg);

    // Recursive descent parsing methods — one per grammar rule
    ASTNode* parseProgram();
    ASTNode* parseStatement();
    ASTNode* parseDeclaration();
    ASTNode* parseAssignment();
    ASTNode* parseIfStatement();
    ASTNode* parseWhileStatement();
    ASTNode* parsePrintStatement();
    ASTNode* parseInputStatement();
    std::vector<ASTNode*> parseBlock();

    // Expression parsing with operator precedence
    ASTNode* parseExpression();     // entry point
    ASTNode* parseLogicOr();        // ||
    ASTNode* parseLogicAnd();       // &&
    ASTNode* parseEquality();       // == !=
    ASTNode* parseComparison();     // < > <= >=
    ASTNode* parseTerm();           // + -
    ASTNode* parseFactor();         // * /
    ASTNode* parseUnary();          // ! -
    ASTNode* parsePrimary();        // literals, identifiers, ( expr )
};

#endif // PARSER_H
