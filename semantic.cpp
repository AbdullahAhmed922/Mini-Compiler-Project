// ============================================================
// semantic.cpp — Semantic Analyzer Implementation
// Person 4's Module
// Description: Walks the AST and performs these checks:
//   1. Variable declared before use
//   2. No duplicate declarations in same scope
//   3. Type compatibility in assignments (int = string → error)
//   4. Type compatibility in expressions (string + int → error)
//   5. Condition in if/while must be numeric (not string)
//   6. print() accepts any type
//   7. input() target must be a declared variable
// ============================================================

#include "semantic.h"

// ==================== Constructor ====================
SemanticAnalyzer::SemanticAnalyzer(SymbolTable& st) : symbolTable(st) {}

// ==================== Main Entry Point ====================
bool SemanticAnalyzer::analyze(ASTNode* root) {
    if (!root) return false;
    analyzeNode(root);
    return errors.empty();
}

std::vector<std::string> SemanticAnalyzer::getErrors() const {
    return errors;
}

void SemanticAnalyzer::addError(const std::string& msg) {
    errors.push_back("[Semantic Error] " + msg);
}

// ==================== Node Dispatcher ====================
void SemanticAnalyzer::analyzeNode(ASTNode* node) {
    if (!node) return;

    switch (node->nodeType) {
        case NodeType::PROGRAM:
            analyzeProgram(static_cast<ProgramNode*>(node));
            break;
        case NodeType::DECLARATION:
            analyzeDeclaration(static_cast<DeclarationNode*>(node));
            break;
        case NodeType::ASSIGNMENT:
            analyzeAssignment(static_cast<AssignmentNode*>(node));
            break;
        case NodeType::IF_STATEMENT:
            analyzeIf(static_cast<IfNode*>(node));
            break;
        case NodeType::WHILE_STATEMENT:
            analyzeWhile(static_cast<WhileNode*>(node));
            break;
        case NodeType::PRINT_STATEMENT:
            analyzePrint(static_cast<PrintNode*>(node));
            break;
        case NodeType::INPUT_STATEMENT:
            analyzeInput(static_cast<InputNode*>(node));
            break;
        default:
            break;
    }
}

// ==================== Statement Analysis ====================

void SemanticAnalyzer::analyzeProgram(ProgramNode* node) {
    for (auto stmt : node->statements) {
        analyzeNode(stmt);
    }
}

void SemanticAnalyzer::analyzeDeclaration(DeclarationNode* node) {
    // Try to insert into symbol table. If it fails, the variable
    // was already declared in this scope.
    if (!symbolTable.insert(node->varName, node->dataType, node->line)) {
        addError("Variable '" + node->varName
                 + "' is already declared in this scope (line "
                 + std::to_string(node->line) + ")");
    }
}

void SemanticAnalyzer::analyzeAssignment(AssignmentNode* node) {
    // Check 1: Is the variable declared?
    Symbol* sym = symbolTable.lookup(node->varName);
    if (!sym) {
        addError("Variable '" + node->varName
                 + "' used before declaration (line "
                 + std::to_string(node->line) + ")");
        return;
    }

    // Check 2: Analyze the RHS expression and get its type
    std::string exprType = analyzeExpression(node->expression);

    // Check 3: Type compatibility
    if (exprType != "error") {
        if (!areTypesCompatible(sym->dataType, exprType)) {
            addError("Type mismatch: cannot assign '" + exprType
                     + "' to '" + sym->dataType + "' variable '"
                     + node->varName + "' (line "
                     + std::to_string(node->line) + ")");
        }
    }

    // Mark the variable as initialized
    symbolTable.setInitialized(node->varName);
}

void SemanticAnalyzer::analyzeIf(IfNode* node) {
    // Check: condition must not be a string
    std::string condType = analyzeExpression(node->condition);
    if (condType == "string") {
        addError("Condition in 'if' statement must be numeric, got 'string' (line "
                 + std::to_string(node->line) + ")");
    }

    // Analyze 'then' body in a new scope
    symbolTable.enterScope();
    for (auto stmt : node->thenBody) {
        analyzeNode(stmt);
    }
    symbolTable.exitScope();

    // Analyze 'else' body (if present) in a new scope
    if (!node->elseBody.empty()) {
        symbolTable.enterScope();
        for (auto stmt : node->elseBody) {
            analyzeNode(stmt);
        }
        symbolTable.exitScope();
    }
}

void SemanticAnalyzer::analyzeWhile(WhileNode* node) {
    // Check: condition must not be a string
    std::string condType = analyzeExpression(node->condition);
    if (condType == "string") {
        addError("Condition in 'while' statement must be numeric, got 'string' (line "
                 + std::to_string(node->line) + ")");
    }

    // Analyze loop body in a new scope
    symbolTable.enterScope();
    for (auto stmt : node->body) {
        analyzeNode(stmt);
    }
    symbolTable.exitScope();
}

void SemanticAnalyzer::analyzePrint(PrintNode* node) {
    // print() accepts any type — just analyze the expression
    analyzeExpression(node->expression);
}

void SemanticAnalyzer::analyzeInput(InputNode* node) {
    // Check: target variable must be declared
    Symbol* sym = symbolTable.lookup(node->varName);
    if (!sym) {
        addError("Variable '" + node->varName
                 + "' used in input() before declaration (line "
                 + std::to_string(node->line) + ")");
    } else {
        symbolTable.setInitialized(node->varName);
    }
}

// ==================== Expression Analysis ====================
// Each method returns the resulting type of the expression.

std::string SemanticAnalyzer::analyzeExpression(ASTNode* node) {
    if (!node) return "error";

    switch (node->nodeType) {
        case NodeType::LITERAL: {
            LiteralNode* lit = static_cast<LiteralNode*>(node);
            if (lit->literalType == TokenType::INTEGER_LITERAL) return "int";
            if (lit->literalType == TokenType::FLOAT_LITERAL)   return "float";
            if (lit->literalType == TokenType::STRING_LITERAL)  return "string";
            return "error";
        }

        case NodeType::IDENTIFIER: {
            IdentifierNode* id = static_cast<IdentifierNode*>(node);
            Symbol* sym = symbolTable.lookup(id->name);
            if (!sym) {
                addError("Variable '" + id->name
                         + "' used before declaration (line "
                         + std::to_string(node->line) + ")");
                return "error";
            }
            return sym->dataType;
        }

        case NodeType::BINARY_EXPR:
            return analyzeBinaryExpr(static_cast<BinaryExprNode*>(node));

        case NodeType::UNARY_EXPR:
            return analyzeUnaryExpr(static_cast<UnaryExprNode*>(node));

        default:
            return "error";
    }
}

std::string SemanticAnalyzer::analyzeBinaryExpr(BinaryExprNode* node) {
    std::string leftType  = analyzeExpression(node->left);
    std::string rightType = analyzeExpression(node->right);

    if (leftType == "error" || rightType == "error") return "error";

    std::string result = getResultType(leftType, rightType, node->op);
    if (result == "error") {
        addError("Invalid operation: '" + leftType + "' " + node->op
                 + " '" + rightType + "' (line "
                 + std::to_string(node->line) + ")");
    }
    return result;
}

std::string SemanticAnalyzer::analyzeUnaryExpr(UnaryExprNode* node) {
    std::string operandType = analyzeExpression(node->operand);
    if (operandType == "error") return "error";

    if (node->op == "!") {
        if (operandType == "string") {
            addError("Cannot apply '!' operator to 'string' type (line "
                     + std::to_string(node->line) + ")");
            return "error";
        }
        return "int";  // logical NOT always returns int (0 or 1)
    }

    if (node->op == "-") {
        if (operandType == "string") {
            addError("Cannot apply unary '-' to 'string' type (line "
                     + std::to_string(node->line) + ")");
            return "error";
        }
        return operandType;  // negation preserves int/float type
    }

    return "error";
}

// ==================== Type System Helpers ====================

bool SemanticAnalyzer::areTypesCompatible(const std::string& target,
                                          const std::string& source) {
    if (target == source) return true;

    // Allow implicit conversion between int and float
    if ((target == "int" && source == "float") ||
        (target == "float" && source == "int")) {
        return true;
    }

    return false;  // string is not compatible with int or float
}

std::string SemanticAnalyzer::getResultType(const std::string& left,
                                            const std::string& right,
                                            const std::string& op) {
    // Comparison and logical operators always return int (as boolean)
    if (op == "==" || op == "!=" || op == "<" || op == ">" ||
        op == "<=" || op == ">=" || op == "&&" || op == "||") {
        // For && and ||, both operands must be numeric
        if (op == "&&" || op == "||") {
            if (left == "string" || right == "string") return "error";
        }
        // For comparison, string can only use == and !=
        if (left == "string" || right == "string") {
            if (op == "==" || op == "!=") {
                if (left == "string" && right == "string") return "int";
                return "error"; // can't compare string with number
            }
            return "error";
        }
        return "int";
    }

    // Arithmetic operators: +, -, *, /
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        // Special case: string + string = string concatenation
        if (left == "string" && right == "string" && op == "+") {
            return "string";
        }
        // No other arithmetic on strings
        if (left == "string" || right == "string") {
            return "error";
        }
        // int/float arithmetic
        if (left == "float" || right == "float") return "float";
        return "int";
    }

    return "error";
}
