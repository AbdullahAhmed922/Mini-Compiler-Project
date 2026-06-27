// ============================================================
// parser.cpp — Parser (Syntax Analyzer) Implementation
// Person 3's Module
// Description: Implements a recursive descent parser that
//              converts a token stream into an Abstract Syntax
//              Tree (AST). Handles operator precedence via
//              layered parsing functions.
//
// Grammar (simplified BNF):
//   program      → statement*
//   statement    → declaration | assignment | if_stmt
//                | while_stmt | print_stmt | input_stmt
//   declaration  → type IDENTIFIER ';'
//   assignment   → IDENTIFIER '=' expression ';'
//   if_stmt      → 'if' '(' expr ')' block ('else' block)?
//   while_stmt   → 'while' '(' expr ')' block
//   print_stmt   → 'print' '(' expr ')' ';'
//   input_stmt   → 'input' '(' IDENTIFIER ')' ';'
//   block        → '{' statement* '}'
//   expression   → logic_or
//   logic_or     → logic_and ('||' logic_and)*
//   logic_and    → equality ('&&' equality)*
//   equality     → comparison (('=='|'!=') comparison)*
//   comparison   → term (('<'|'>'|'<='|'>=') term)*
//   term         → factor (('+'|'-') factor)*
//   factor       → unary (('*'|'/') unary)*
//   unary        → ('!'|'-') unary | primary
//   primary      → NUMBER | FLOAT | STRING | IDENTIFIER | '(' expr ')'
// ============================================================

#include "parser.h"
#include <iostream>

// ==================== AST Print Helpers ====================

static void printIndent(int indent) {
    for (int i = 0; i < indent; i++) std::cout << "  ";
}

// ==================== AST Node Destructors & Print ====================

ProgramNode::~ProgramNode() {
    for (auto s : statements) delete s;
}
void ProgramNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Program" << std::endl;
    for (auto s : statements) s->print(indent + 1);
}

void DeclarationNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Declare: " << dataType << " " << varName << std::endl;
}

AssignmentNode::~AssignmentNode() { delete expression; }
void AssignmentNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Assign: " << varName << " =" << std::endl;
    if (expression) expression->print(indent + 1);
}

IfNode::~IfNode() {
    delete condition;
    for (auto s : thenBody) delete s;
    for (auto s : elseBody) delete s;
}
void IfNode::print(int indent) const {
    printIndent(indent);
    std::cout << "If:" << std::endl;
    printIndent(indent + 1); std::cout << "Condition:" << std::endl;
    if (condition) condition->print(indent + 2);
    printIndent(indent + 1); std::cout << "Then:" << std::endl;
    for (auto s : thenBody) s->print(indent + 2);
    if (!elseBody.empty()) {
        printIndent(indent + 1); std::cout << "Else:" << std::endl;
        for (auto s : elseBody) s->print(indent + 2);
    }
}

WhileNode::~WhileNode() {
    delete condition;
    for (auto s : body) delete s;
}
void WhileNode::print(int indent) const {
    printIndent(indent);
    std::cout << "While:" << std::endl;
    printIndent(indent + 1); std::cout << "Condition:" << std::endl;
    if (condition) condition->print(indent + 2);
    printIndent(indent + 1); std::cout << "Body:" << std::endl;
    for (auto s : body) s->print(indent + 2);
}

PrintNode::~PrintNode() { delete expression; }
void PrintNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Print:" << std::endl;
    if (expression) expression->print(indent + 1);
}

void InputNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Input: " << varName << std::endl;
}

BinaryExprNode::~BinaryExprNode() { delete left; delete right; }
void BinaryExprNode::print(int indent) const {
    printIndent(indent);
    std::cout << "BinaryExpr: " << op << std::endl;
    if (left) left->print(indent + 1);
    if (right) right->print(indent + 1);
}

UnaryExprNode::~UnaryExprNode() { delete operand; }
void UnaryExprNode::print(int indent) const {
    printIndent(indent);
    std::cout << "UnaryExpr: " << op << std::endl;
    if (operand) operand->print(indent + 1);
}

void LiteralNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Literal: " << value
              << " (" << tokenTypeToString(literalType) << ")" << std::endl;
}

void IdentifierNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Identifier: " << name << std::endl;
}


// ==================== Parser Implementation ====================

Parser::Parser(const std::vector<Token>& t) : tokens(t), pos(0) {}

// ---- Token Access Helpers ----

Token Parser::currentToken() {
    if (pos < (int)tokens.size()) return tokens[pos];
    return Token(TokenType::END_OF_FILE, "", 0);
}

Token Parser::peekToken() {
    if (pos + 1 < (int)tokens.size()) return tokens[pos + 1];
    return Token(TokenType::END_OF_FILE, "", 0);
}

void Parser::advance() {
    if (pos < (int)tokens.size()) pos++;
}

bool Parser::match(TokenType type) {
    if (currentToken().type == type) {
        advance();
        return true;
    }
    return false;
}

bool Parser::expect(TokenType type, const std::string& errMsg) {
    if (currentToken().type == type) {
        advance();
        return true;
    }
    addError(errMsg + " at line " + std::to_string(currentToken().line)
             + " (got '" + currentToken().value + "')");
    return false;
}

void Parser::addError(const std::string& msg) {
    errors.push_back("[Parser Error] " + msg);
}

bool Parser::hasErrors() const {
    return !errors.empty();
}

std::vector<std::string> Parser::getErrors() const {
    return errors;
}

// ---- Entry Point ----

ASTNode* Parser::parse() {
    return parseProgram();
}

// ---- Grammar Rules ----

ASTNode* Parser::parseProgram() {
    ProgramNode* program = new ProgramNode();

    while (currentToken().type != TokenType::END_OF_FILE) {
        ASTNode* stmt = parseStatement();
        if (stmt) {
            program->statements.push_back(stmt);
        } else {
            // Error recovery: skip the current token
            advance();
        }
    }

    return program;
}

ASTNode* Parser::parseStatement() {
    TokenType type = currentToken().type;

    // Variable declaration: type identifier ;
    if (type == TokenType::KW_INT || type == TokenType::KW_FLOAT ||
        type == TokenType::KW_STRING) {
        return parseDeclaration();
    }

    // Assignment: identifier = expression ;
    if (type == TokenType::IDENTIFIER && peekToken().type == TokenType::ASSIGN) {
        return parseAssignment();
    }

    // If statement
    if (type == TokenType::KW_IF) {
        return parseIfStatement();
    }

    // While statement
    if (type == TokenType::KW_WHILE) {
        return parseWhileStatement();
    }

    // Print statement
    if (type == TokenType::KW_PRINT) {
        return parsePrintStatement();
    }

    // Input statement
    if (type == TokenType::KW_INPUT) {
        return parseInputStatement();
    }

    addError("Unexpected token '" + currentToken().value
             + "' at line " + std::to_string(currentToken().line));
    return nullptr;
}

ASTNode* Parser::parseDeclaration() {
    int ln = currentToken().line;
    std::string dataType = currentToken().value;  // "int", "float", or "string"
    advance(); // consume type keyword

    if (currentToken().type != TokenType::IDENTIFIER) {
        addError("Expected identifier after type '" + dataType
                 + "' at line " + std::to_string(ln));
        return nullptr;
    }

    std::string varName = currentToken().value;
    advance(); // consume identifier

    expect(TokenType::SEMICOLON, "Expected ';' after declaration of '" + varName + "'");

    return new DeclarationNode(dataType, varName, ln);
}

ASTNode* Parser::parseAssignment() {
    int ln = currentToken().line;
    std::string varName = currentToken().value;
    advance(); // consume identifier
    advance(); // consume '='

    ASTNode* expr = parseExpression();
    if (!expr) return nullptr;

    expect(TokenType::SEMICOLON, "Expected ';' after assignment to '" + varName + "'");

    return new AssignmentNode(varName, expr, ln);
}

ASTNode* Parser::parseIfStatement() {
    int ln = currentToken().line;
    advance(); // consume 'if'

    expect(TokenType::LPAREN, "Expected '(' after 'if'");

    ASTNode* condition = parseExpression();
    if (!condition) return nullptr;

    expect(TokenType::RPAREN, "Expected ')' after if condition");

    IfNode* ifNode = new IfNode(condition, ln);
    ifNode->thenBody = parseBlock();

    // Optional else clause
    if (currentToken().type == TokenType::KW_ELSE) {
        advance(); // consume 'else'
        ifNode->elseBody = parseBlock();
    }

    return ifNode;
}

ASTNode* Parser::parseWhileStatement() {
    int ln = currentToken().line;
    advance(); // consume 'while'

    expect(TokenType::LPAREN, "Expected '(' after 'while'");

    ASTNode* condition = parseExpression();
    if (!condition) return nullptr;

    expect(TokenType::RPAREN, "Expected ')' after while condition");

    WhileNode* whileNode = new WhileNode(condition, ln);
    whileNode->body = parseBlock();

    return whileNode;
}

ASTNode* Parser::parsePrintStatement() {
    int ln = currentToken().line;
    advance(); // consume 'print'

    expect(TokenType::LPAREN, "Expected '(' after 'print'");

    ASTNode* expr = parseExpression();
    if (!expr) return nullptr;

    expect(TokenType::RPAREN, "Expected ')' after print argument");
    expect(TokenType::SEMICOLON, "Expected ';' after print statement");

    return new PrintNode(expr, ln);
}

ASTNode* Parser::parseInputStatement() {
    int ln = currentToken().line;
    advance(); // consume 'input'

    expect(TokenType::LPAREN, "Expected '(' after 'input'");

    if (currentToken().type != TokenType::IDENTIFIER) {
        addError("Expected variable name in input() at line " + std::to_string(ln));
        return nullptr;
    }

    std::string varName = currentToken().value;
    advance(); // consume identifier

    expect(TokenType::RPAREN, "Expected ')' after input argument");
    expect(TokenType::SEMICOLON, "Expected ';' after input statement");

    return new InputNode(varName, ln);
}

std::vector<ASTNode*> Parser::parseBlock() {
    std::vector<ASTNode*> stmts;

    expect(TokenType::LBRACE, "Expected '{'");

    while (currentToken().type != TokenType::RBRACE &&
           currentToken().type != TokenType::END_OF_FILE) {
        ASTNode* stmt = parseStatement();
        if (stmt) {
            stmts.push_back(stmt);
        } else {
            advance(); // error recovery — skip bad token
        }
    }

    expect(TokenType::RBRACE, "Expected '}'");

    return stmts;
}


// ==================== Expression Parsing ====================
// Uses precedence climbing: each function handles one
// precedence level and delegates to the next tighter level.

ASTNode* Parser::parseExpression() {
    return parseLogicOr();
}

// Lowest precedence: ||
ASTNode* Parser::parseLogicOr() {
    ASTNode* left = parseLogicAnd();

    while (currentToken().type == TokenType::OR) {
        std::string op = currentToken().value;
        int ln = currentToken().line;
        advance();
        ASTNode* right = parseLogicAnd();
        left = new BinaryExprNode(op, left, right, ln);
    }

    return left;
}

// &&
ASTNode* Parser::parseLogicAnd() {
    ASTNode* left = parseEquality();

    while (currentToken().type == TokenType::AND) {
        std::string op = currentToken().value;
        int ln = currentToken().line;
        advance();
        ASTNode* right = parseEquality();
        left = new BinaryExprNode(op, left, right, ln);
    }

    return left;
}

// == !=
ASTNode* Parser::parseEquality() {
    ASTNode* left = parseComparison();

    while (currentToken().type == TokenType::EQUAL ||
           currentToken().type == TokenType::NOT_EQUAL) {
        std::string op = currentToken().value;
        int ln = currentToken().line;
        advance();
        ASTNode* right = parseComparison();
        left = new BinaryExprNode(op, left, right, ln);
    }

    return left;
}

// < > <= >=
ASTNode* Parser::parseComparison() {
    ASTNode* left = parseTerm();

    while (currentToken().type == TokenType::LESS ||
           currentToken().type == TokenType::GREATER ||
           currentToken().type == TokenType::LESS_EQ ||
           currentToken().type == TokenType::GREATER_EQ) {
        std::string op = currentToken().value;
        int ln = currentToken().line;
        advance();
        ASTNode* right = parseTerm();
        left = new BinaryExprNode(op, left, right, ln);
    }

    return left;
}

// + -
ASTNode* Parser::parseTerm() {
    ASTNode* left = parseFactor();

    while (currentToken().type == TokenType::PLUS ||
           currentToken().type == TokenType::MINUS) {
        std::string op = currentToken().value;
        int ln = currentToken().line;
        advance();
        ASTNode* right = parseFactor();
        left = new BinaryExprNode(op, left, right, ln);
    }

    return left;
}

// * /
ASTNode* Parser::parseFactor() {
    ASTNode* left = parseUnary();

    while (currentToken().type == TokenType::MULTIPLY ||
           currentToken().type == TokenType::DIVIDE) {
        std::string op = currentToken().value;
        int ln = currentToken().line;
        advance();
        ASTNode* right = parseUnary();
        left = new BinaryExprNode(op, left, right, ln);
    }

    return left;
}

// Unary: ! and - (prefix)
ASTNode* Parser::parseUnary() {
    if (currentToken().type == TokenType::NOT ||
        currentToken().type == TokenType::MINUS) {
        std::string op = currentToken().value;
        int ln = currentToken().line;
        advance();
        ASTNode* operand = parseUnary(); // recursive for chained unaries
        return new UnaryExprNode(op, operand, ln);
    }

    return parsePrimary();
}

// Highest precedence: literals, identifiers, parenthesized expressions
ASTNode* Parser::parsePrimary() {
    Token tok = currentToken();

    // Number or string literal
    if (tok.type == TokenType::INTEGER_LITERAL ||
        tok.type == TokenType::FLOAT_LITERAL ||
        tok.type == TokenType::STRING_LITERAL) {
        advance();
        return new LiteralNode(tok.value, tok.type, tok.line);
    }

    // Variable reference
    if (tok.type == TokenType::IDENTIFIER) {
        advance();
        return new IdentifierNode(tok.value, tok.line);
    }

    // Parenthesized sub-expression
    if (tok.type == TokenType::LPAREN) {
        advance(); // consume '('
        ASTNode* expr = parseExpression();
        expect(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    addError("Expected expression at line " + std::to_string(tok.line)
             + " (got '" + tok.value + "')");
    return nullptr;
}
