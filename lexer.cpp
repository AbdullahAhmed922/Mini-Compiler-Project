// ============================================================
// lexer.cpp — Lexical Analyzer Implementation
// Person 1's Module
// Description: Scans the raw source code character by character
//              and produces a vector of Token objects.
// Recognized elements:
//   - Keywords: int, float, string, if, else, while, print, input
//   - Literals: integers, floats, strings
//   - Operators: +, -, *, /, =, ==, !=, <, >, <=, >=, &&, ||, !
//   - Delimiters: ; ( ) { } ,
//   - Comments: // (single-line, skipped)
// ============================================================

#include "lexer.h"
#include <cctype>
#include <sstream>
#include <iomanip>

// ==================== Constructor ====================
Lexer::Lexer(const std::string& sourceCode)
    : source(sourceCode), pos(0), line(1) {}

// ==================== Character Access ====================

char Lexer::currentChar() {
    if (pos < (int)source.length()) return source[pos];
    return '\0';
}

char Lexer::peek() {
    if (pos + 1 < (int)source.length()) return source[pos + 1];
    return '\0';
}

void Lexer::advance() {
    if (pos < (int)source.length()) {
        if (source[pos] == '\n') line++;
        pos++;
    }
}

// ==================== Skip Helpers ====================

void Lexer::skipWhitespace() {
    while (pos < (int)source.length() && isspace(currentChar())) {
        advance();
    }
}

void Lexer::skipComment() {
    // Single-line comment: skip until end of line
    if (currentChar() == '/' && peek() == '/') {
        while (pos < (int)source.length() && currentChar() != '\n') {
            advance();
        }
    }
}

// ==================== Read Token Helpers ====================

Token Lexer::readNumber() {
    std::string num = "";
    bool isFloat = false;
    int startLine = line;

    while (pos < (int)source.length() && (isdigit(currentChar()) || currentChar() == '.')) {
        if (currentChar() == '.') {
            if (isFloat) {
                addError("Invalid number format: multiple decimal points at line " + std::to_string(line));
                break;
            }
            isFloat = true;
        }
        num += currentChar();
        advance();
    }

    if (isFloat)
        return Token(TokenType::FLOAT_LITERAL, num, startLine);
    else
        return Token(TokenType::INTEGER_LITERAL, num, startLine);
}

Token Lexer::readString() {
    std::string str = "";
    int startLine = line;
    advance(); // skip the opening double-quote

    while (pos < (int)source.length() && currentChar() != '"') {
        if (currentChar() == '\n') {
            addError("Unterminated string literal at line " + std::to_string(startLine));
            return Token(TokenType::STRING_LITERAL, str, startLine);
        }
        str += currentChar();
        advance();
    }

    if (pos >= (int)source.length()) {
        addError("Unterminated string literal at line " + std::to_string(startLine));
    } else {
        advance(); // skip the closing double-quote
    }

    return Token(TokenType::STRING_LITERAL, str, startLine);
}

Token Lexer::readIdentifierOrKeyword() {
    std::string id = "";
    int startLine = line;

    while (pos < (int)source.length() && (isalnum(currentChar()) || currentChar() == '_')) {
        id += currentChar();
        advance();
    }

    // Check if the identifier is a reserved keyword
    if (id == "int")    return Token(TokenType::KW_INT,    id, startLine);
    if (id == "float")  return Token(TokenType::KW_FLOAT,  id, startLine);
    if (id == "string") return Token(TokenType::KW_STRING, id, startLine);
    if (id == "if")     return Token(TokenType::KW_IF,     id, startLine);
    if (id == "else")   return Token(TokenType::KW_ELSE,   id, startLine);
    if (id == "while")  return Token(TokenType::KW_WHILE,  id, startLine);
    if (id == "print")  return Token(TokenType::KW_PRINT,  id, startLine);
    if (id == "input")  return Token(TokenType::KW_INPUT,  id, startLine);

    // Not a keyword, so it's a user-defined identifier
    return Token(TokenType::IDENTIFIER, id, startLine);
}

// ==================== Error Reporting ====================

void Lexer::addError(const std::string& msg) {
    errors.push_back("[Lexer Error] " + msg);
}

bool Lexer::hasErrors() const {
    return !errors.empty();
}

std::vector<std::string> Lexer::getErrors() const {
    return errors;
}

// ==================== Main Tokenization ====================

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < (int)source.length()) {
        // Skip whitespace
        skipWhitespace();
        if (pos >= (int)source.length()) break;

        // Skip single-line comments
        if (currentChar() == '/' && peek() == '/') {
            skipComment();
            continue;
        }

        char ch = currentChar();

        // --- Numbers ---
        if (isdigit(ch)) {
            tokens.push_back(readNumber());
            continue;
        }

        // --- String literals ---
        if (ch == '"') {
            tokens.push_back(readString());
            continue;
        }

        // --- Identifiers and keywords ---
        if (isalpha(ch) || ch == '_') {
            tokens.push_back(readIdentifierOrKeyword());
            continue;
        }

        // --- Operators and Delimiters ---
        int startLine = line;

        switch (ch) {
            // Arithmetic
            case '+':
                tokens.push_back(Token(TokenType::PLUS, "+", startLine));
                advance();
                break;
            case '-':
                tokens.push_back(Token(TokenType::MINUS, "-", startLine));
                advance();
                break;
            case '*':
                tokens.push_back(Token(TokenType::MULTIPLY, "*", startLine));
                advance();
                break;
            case '/':
                tokens.push_back(Token(TokenType::DIVIDE, "/", startLine));
                advance();
                break;

            // Assignment or Equality
            case '=':
                if (peek() == '=') {
                    tokens.push_back(Token(TokenType::EQUAL, "==", startLine));
                    advance(); advance();
                } else {
                    tokens.push_back(Token(TokenType::ASSIGN, "=", startLine));
                    advance();
                }
                break;

            // Not / Not-Equal
            case '!':
                if (peek() == '=') {
                    tokens.push_back(Token(TokenType::NOT_EQUAL, "!=", startLine));
                    advance(); advance();
                } else {
                    tokens.push_back(Token(TokenType::NOT, "!", startLine));
                    advance();
                }
                break;

            // Less / Less-Equal
            case '<':
                if (peek() == '=') {
                    tokens.push_back(Token(TokenType::LESS_EQ, "<=", startLine));
                    advance(); advance();
                } else {
                    tokens.push_back(Token(TokenType::LESS, "<", startLine));
                    advance();
                }
                break;

            // Greater / Greater-Equal
            case '>':
                if (peek() == '=') {
                    tokens.push_back(Token(TokenType::GREATER_EQ, ">=", startLine));
                    advance(); advance();
                } else {
                    tokens.push_back(Token(TokenType::GREATER, ">", startLine));
                    advance();
                }
                break;

            // Logical AND
            case '&':
                if (peek() == '&') {
                    tokens.push_back(Token(TokenType::AND, "&&", startLine));
                    advance(); advance();
                } else {
                    addError("Unexpected character '&' at line " + std::to_string(line) + ". Did you mean '&&'?");
                    advance();
                }
                break;

            // Logical OR
            case '|':
                if (peek() == '|') {
                    tokens.push_back(Token(TokenType::OR, "||", startLine));
                    advance(); advance();
                } else {
                    addError("Unexpected character '|' at line " + std::to_string(line) + ". Did you mean '||'?");
                    advance();
                }
                break;

            // Delimiters
            case ';':
                tokens.push_back(Token(TokenType::SEMICOLON, ";", startLine));
                advance();
                break;
            case '(':
                tokens.push_back(Token(TokenType::LPAREN, "(", startLine));
                advance();
                break;
            case ')':
                tokens.push_back(Token(TokenType::RPAREN, ")", startLine));
                advance();
                break;
            case '{':
                tokens.push_back(Token(TokenType::LBRACE, "{", startLine));
                advance();
                break;
            case '}':
                tokens.push_back(Token(TokenType::RBRACE, "}", startLine));
                advance();
                break;
            case ',':
                tokens.push_back(Token(TokenType::COMMA, ",", startLine));
                advance();
                break;

            // Unknown character
            default:
                addError("Unexpected character '" + std::string(1, ch) + "' at line " + std::to_string(line));
                advance();
                break;
        }
    }

    // Append end-of-file token
    tokens.push_back(Token(TokenType::END_OF_FILE, "", line));
    return tokens;
}
