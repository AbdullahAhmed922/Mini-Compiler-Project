// ============================================================
// icg.cpp — Intermediate Code Generator Implementation
// Person 5's Module
// Description: Walks the AST and produces Three-Address Code.
//
// Generated TAC instrauctions include:
//   DECLARE  type  -  varName      → variable declaration
//   =        value -  varName      → assignment
//   op       arg1  arg2  result    → aarithmetic/comparison
//   IF_FALSE cond  -  label        → conditional jump
//   GOTO     -     -  label        → unconditional jump
//   LABEL    -     -  labelName    → label marker
//   PRINT    value -  -            → output
//   INPUT    -     -  varName      → input
//
// Temporary variables: t1, t2, t3, ...
// Labels: L1, L2, L3, ...
// ============================================uu================

#include "icg.h"
#include <iostream>
#include <iomanip>

// ==================== Constructor ====================
ICG::ICG() : tempCount(0), labelCount(0) {}

// ==================== Temp & Label Generators ====================

std::string ICG::newTemp() {
    return "t" + std::to_string(++tempCount);
}

std::string ICG::newLabel() {
    return "L" + std::to_string(++labelCount);
}

// ==================== Emit ====================
void ICG::emit(const std::string& op, const std::string& arg1,
               const std::string& arg2, const std::string& result) {
    code.push_back(TAC(op, arg1, arg2, result));
}

// ==================== Main Entry Point ====================

void ICG::generate(ASTNode* root) {
    if (!root) return;
    generateNode(root);
}

std::vector<TAC> ICG::getCode() const {
    return code;
}

// ==================== Print TAC ====================
void ICG::printCode() const {
    // --- Table format ---
    std::cout << std::endl;
    std::cout << "  +-----+------------+----------+----------+----------+" << std::endl;
    std::cout << "  | " << std::left << std::setw(3) << "#"
              << " | " << std::setw(10) << "Op"
              << " | " << std::setw(8) << "Arg1"
              << " | " << std::setw(8) << "Arg2"
              << " | " << std::setw(8) << "Result"
              << " |" << std::endl;
    std::cout << "  +-----+------------+----------+----------+----------+" << std::endl;

    int lineNum = 1;
    for (const auto& tac : code) {
        std::cout << "  | " << std::left << std::setw(3) << lineNum++
                  << " | " << std::setw(10) << tac.op
                  << " | " << std::setw(8) << tac.arg1
                  << " | " << std::setw(8) << tac.arg2
                  << " | " << std::setw(8) << tac.result
                  << " |" << std::endl;
    }
    std::cout << "  +-----+------------+----------+----------+----------+" << std::endl;

    // --- Human-readable format ---
    std::cout << std::endl;
    std::cout << "  Readable Three-Address Code:" << std::endl;
    std::cout << "  " << std::string(40, '-') << std::endl;

    for (const auto& tac : code) {
        if (tac.op == "LABEL") {
            std::cout << "  " << tac.result << ":" << std::endl;
        }
        else if (tac.op == "GOTO") {
            std::cout << "      GOTO " << tac.result << std::endl;
        }
        else if (tac.op == "IF_FALSE") {
            std::cout << "      IF_FALSE " << tac.arg1 << " GOTO " << tac.result << std::endl;
        }
        else if (tac.op == "PRINT") {
            std::cout << "      PRINT " << tac.arg1 << std::endl;
        }
        else if (tac.op == "INPUT") {
            std::cout << "      INPUT " << tac.result << std::endl;
        }
        else if (tac.op == "DECLARE") {
            std::cout << "      DECLARE " << tac.result << " : " << tac.arg1 << std::endl;
        }
        else if (tac.op == "=") {
            std::cout << "      " << tac.result << " = " << tac.arg1 << std::endl;
        }
        else {
            // Binary or unary operation
            if (tac.arg2.empty()) {
                // Unary: result = op arg1
                std::cout << "      " << tac.result << " = " << tac.op << " " << tac.arg1 << std::endl;
            } else {
                // Binary: result = arg1 op arg2
                std::cout << "      " << tac.result << " = " << tac.arg1 << " " << tac.op << " " << tac.arg2 << std::endl;
            }
        }
    }
    std::cout << "  " << std::string(40, '-') << std::endl;
}

// ==================== Node Dispatcher ====================

void ICG::generateNode(ASTNode* node) {
    if (!node) return;

    switch (node->nodeType) {
        case NodeType::PROGRAM:
            generateProgram(static_cast<ProgramNode*>(node));
            break;
        case NodeType::DECLARATION:
            generateDeclaration(static_cast<DeclarationNode*>(node));
            break;
        case NodeType::ASSIGNMENT:
            generateAssignment(static_cast<AssignmentNode*>(node));
            break;
        case NodeType::IF_STATEMENT:
            generateIf(static_cast<IfNode*>(node));
            break;
        case NodeType::WHILE_STATEMENT:
            generateWhile(static_cast<WhileNode*>(node));
            break;
        case NodeType::PRINT_STATEMENT:
            generatePrint(static_cast<PrintNode*>(node));
            break;
        case NodeType::INPUT_STATEMENT:
            generateInput(static_cast<InputNode*>(node));
            break;
        default:
            break;
    }
}

// ==================== Statement Generation ====================

void ICG::generateProgram(ProgramNode* node) {
    for (auto stmt : node->statements) {
        generateNode(stmt);
    }
}

void ICG::generateDeclaration(DeclarationNode* node) {
    // Emit: DECLARE type - varName
    emit("DECLARE", node->dataType, "", node->varName);
}

void ICG::generateAssignment(AssignmentNode* node) {
    // Generate code for the RHS expression
    std::string exprResult = generateExpression(node->expression);
    // Emit: = exprResult - varName
    emit("=", exprResult, "", node->varName);
}

void ICG::generateIf(IfNode* node) {
    // Generate code for condition
    std::string condResult = generateExpression(node->condition);

    std::string labelElse = newLabel();
    std::string labelEnd  = newLabel();

    if (!node->elseBody.empty()) {
        // IF_FALSE cond GOTO labelElse
        emit("IF_FALSE", condResult, "", labelElse);

        // Generate 'then' body
        for (auto stmt : node->thenBody) {
            generateNode(stmt);
        }
        // Jump over else
        emit("GOTO", "", "", labelEnd);

        // Else body
        emit("LABEL", "", "", labelElse);
        for (auto stmt : node->elseBody) {
            generateNode(stmt);
        }
        emit("LABEL", "", "", labelEnd);
    } else {
        // No else — jump to end if false
        emit("IF_FALSE", condResult, "", labelEnd);

        // Generate 'then' body
        for (auto stmt : node->thenBody) {
            generateNode(stmt);
        }
        emit("LABEL", "", "", labelEnd);
    }
}

void ICG::generateWhile(WhileNode* node) {
    std::string labelStart = newLabel();
    std::string labelEnd   = newLabel();

    // Loop start label
    emit("LABEL", "", "", labelStart);

    // Evaluate condition
    std::string condResult = generateExpression(node->condition);
    emit("IF_FALSE", condResult, "", labelEnd);

    // Loop body
    for (auto stmt : node->body) {
        generateNode(stmt);
    }

    // Jump back to start
    emit("GOTO", "", "", labelStart);

    // Loop end label
    emit("LABEL", "", "", labelEnd);
}

void ICG::generatePrint(PrintNode* node) {
    std::string exprResult = generateExpression(node->expression);
    emit("PRINT", exprResult, "", "");
}

void ICG::generateInput(InputNode* node) {
    emit("INPUT", "", "", node->varName);
}

// ==================== Expression Generation ====================

std::string ICG::generateExpression(ASTNode* node) {
    if (!node) return "";

    switch (node->nodeType) {
        case NodeType::LITERAL: {
            LiteralNode* lit = static_cast<LiteralNode*>(node);
            return lit->value;  // return the literal value directly
        }

        case NodeType::IDENTIFIER: {
            IdentifierNode* id = static_cast<IdentifierNode*>(node);
            return id->name;    // return the variable name directly
        }

        case NodeType::BINARY_EXPR:
            return generateBinaryExpr(static_cast<BinaryExprNode*>(node));

        case NodeType::UNARY_EXPR:
            return generateUnaryExpr(static_cast<UnaryExprNode*>(node));

        default:
            return "";
    }
}

std::string ICG::generateBinaryExpr(BinaryExprNode* node) {
    // Generate code for left and right sub-expressions
    std::string left  = generateExpression(node->left);
    std::string right = generateExpression(node->right);

    // Create a new temp to hold the result
    std::string temp = newTemp();

    // Emit: op left right temp
    emit(node->op, left, right, temp);

    return temp;
}

std::string ICG::generateUnaryExpr(UnaryExprNode* node) {
    // Generate code for the operand
    std::string operand = generateExpression(node->operand);

    // Create a new temp to hold the result
    std::string temp = newTemp();

    // Emit: op operand - temp
    emit(node->op, operand, "", temp);

    return temp;
}
