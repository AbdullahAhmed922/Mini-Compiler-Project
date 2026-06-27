// ============================================================
// main.cpp — Main Driver & Integration
// Person 6's Module
// Description: Wires all compiler phases together:
//   1. Read source file
//   2. Lexical Analysis  → tokens
//   3. Syntax Analysis   → AST
//   4. Semantic Analysis  → type/scope checks
//   5. ICG               → Three-Address Code
//
//   Provides color-coded console output and error handling.
//   Usage: compiler <source_file>
//      or: compiler              (reads from input.txt)
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

#include "token.h"
#include "lexer.h"
#include "symbol_table.h"
#include "parser.h"
#include "semantic.h"
#include "icg.h"

// ==================== Windows Console Color Support ====================
#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
void enableColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#else
void enableColors() {}
#endif

// ==================== ANSI Color Codes ====================
const std::string RESET   = "\033[0m";
const std::string RED     = "\033[1;31m";
const std::string GREEN   = "\033[1;32m";
const std::string YELLOW  = "\033[1;33m";
const std::string BLUE    = "\033[1;34m";
const std::string MAGENTA = "\033[1;35m";
const std::string CYAN    = "\033[1;36m";
const std::string WHITE   = "\033[1;37m";

// ==================== Output Helpers ====================

void printBanner() {
    std::cout << CYAN;
    std::cout << "========================================================" << std::endl;
    std::cout << "            MINI COMPILER - C++ Implementation          " << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << RESET << std::endl;
}

void printPhaseHeader(const std::string& phase) {
    std::cout << std::endl;
    std::cout << YELLOW;
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "  Phase: " << phase << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << RESET;
}

void printSuccess(const std::string& msg) {
    std::cout << GREEN << "  [OK] " << msg << RESET << std::endl;
}

void printError(const std::string& msg) {
    std::cout << RED << "  [ERROR] " << msg << RESET << std::endl;
}

void printErrors(const std::vector<std::string>& errors) {
    for (const auto& err : errors) {
        printError(err);
    }
}

// Print tokens in a formatted table
void printTokens(const std::vector<Token>& tokens) {
    std::cout << std::endl;
    std::cout << "  +" << std::string(6, '-') << "+"
              << std::string(22, '-') << "+"
              << std::string(20, '-') << "+" << std::endl;
    std::cout << "  | " << std::left << std::setw(4) << "Line"
              << " | " << std::setw(20) << "Token Type"
              << " | " << std::setw(18) << "Value"
              << " |" << std::endl;
    std::cout << "  +" << std::string(6, '-') << "+"
              << std::string(22, '-') << "+"
              << std::string(20, '-') << "+" << std::endl;

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::END_OF_FILE) continue;
        std::cout << "  | " << std::left << std::setw(4) << tok.line
                  << " | " << std::setw(20) << tokenTypeToString(tok.type)
                  << " | " << std::setw(18) << tok.value
                  << " |" << std::endl;
    }

    std::cout << "  +" << std::string(6, '-') << "+"
              << std::string(22, '-') << "+"
              << std::string(20, '-') << "+" << std::endl;
}

// ==================== MAIN ====================

int main(int argc, char* argv[]) {
    enableColors();
    printBanner();

    // ------ Read source file ------
    std::string filename = "input.txt";
    if (argc > 1) {
        filename = argv[1];
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        printError("Cannot open file: " + filename);
        std::cout << std::endl;
        std::cout << "  Usage: compiler <source_file>" << std::endl;
        std::cout << "     or: compiler              (reads from input.txt)" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceCode = buffer.str();
    file.close();

    // ------ Display source code ------
    std::cout << BLUE << "  Source file: " << filename << RESET << std::endl;
    std::cout << BLUE << "  Source code:" << RESET << std::endl;
    std::cout << "  " << std::string(44, '-') << std::endl;

    std::istringstream sourceStream(sourceCode);
    std::string line;
    int lineNum = 1;
    while (std::getline(sourceStream, line)) {
        std::cout << "  " << std::setw(3) << lineNum++ << " | " << line << std::endl;
    }
    std::cout << "  " << std::string(44, '-') << std::endl;

    bool hasError = false;

    // ======================== PHASE 1 ========================
    // Lexical Analysis (Person 1's module)
    printPhaseHeader("LEXICAL ANALYSIS (Tokenization)");

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    if (lexer.hasErrors()) {
        printErrors(lexer.getErrors());
        hasError = true;
    } else {
        printSuccess("Lexical analysis completed successfully!");
    }

    std::cout << MAGENTA << "\n  Total tokens: " << (tokens.size() - 1)
              << RESET << std::endl;  // -1 to exclude EOF token
    printTokens(tokens);

    if (hasError) {
        std::cout << RED << "\n  Compilation stopped due to lexical errors."
                  << RESET << std::endl;
        return 1;
    }

    // ======================== PHASE 2 ========================
    // Syntax Analysis / Parsing (Person 3's module)
    printPhaseHeader("SYNTAX ANALYSIS (Parsing)");

    Parser parser(tokens);
    ASTNode* ast = parser.parse();

    if (parser.hasErrors()) {
        printErrors(parser.getErrors());
        hasError = true;
    } else {
        printSuccess("Syntax analysis completed successfully!");
    }

    if (ast) {
        std::cout << MAGENTA << "\n  Abstract Syntax Tree:" << RESET << std::endl;
        std::cout << "  " << std::string(44, '-') << std::endl;
        ast->print(2);
        std::cout << "  " << std::string(44, '-') << std::endl;
    }

    if (hasError) {
        std::cout << RED << "\n  Compilation stopped due to syntax errors."
                  << RESET << std::endl;
        delete ast;
        return 1;
    }

    // ======================== PHASE 3 ========================
    // Semantic Analysis (Person 2 + Person 4's modules)
    printPhaseHeader("SEMANTIC ANALYSIS");

    SymbolTable symbolTable;
    SemanticAnalyzer semanticAnalyzer(symbolTable);
    bool semanticOk = semanticAnalyzer.analyze(ast);

    if (!semanticOk) {
        printErrors(semanticAnalyzer.getErrors());
        hasError = true;
    } else {
        printSuccess("Semantic analysis completed successfully!");
    }

    std::cout << MAGENTA << "\n  Symbol Table:" << RESET << std::endl;
    symbolTable.display();

    if (hasError) {
        std::cout << RED << "\n  Compilation stopped due to semantic errors."
                  << RESET << std::endl;
        delete ast;
        return 1;
    }

    // ======================== PHASE 4 ========================
    // Intermediate Code Generation (Person 5's module)
    printPhaseHeader("INTERMEDIATE CODE GENERATION (Three-Address Code)");

    ICG icg;
    icg.generate(ast);

    printSuccess("Intermediate code generated successfully!");

    std::cout << MAGENTA << "\n  Three-Address Code:" << RESET << std::endl;
    icg.printCode();

    // ======================== DONE ========================
    std::cout << std::endl;
    std::cout << GREEN;
    std::cout << "========================================================" << std::endl;
    std::cout << "          COMPILATION COMPLETED SUCCESSFULLY!            " << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << RESET << std::endl;

    // Cleanup
    delete ast;

#ifdef _WIN32
    system("pause");
#endif

    return 0;
}
