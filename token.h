// ============================================================
// token.h — Shared Token Definitions
// Used by: ALL team members
// Description: Defines TokenType enum, Token struct, and
//              a utility function for debugging output.
// ============================================================

#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

// ==================== Token Types ====================
enum class TokenType {
    // Literals
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,

    // Identifier
    IDENTIFIER,

    // Keywords
    KW_INT,
    KW_FLOAT,
    KW_STRING,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_PRINT,
    KW_INPUT,

    // Arithmetic Operators
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    ASSIGN,         // =

    // Comparison Operators
    EQUAL,          // ==
    NOT_EQUAL,      // !=
    LESS,           // <
    GREATER,        // >
    LESS_EQ,        // <=
    GREATER_EQ,     // >=

    // Logical Operators
    AND,            // &&
    OR,             // ||
    NOT,            // !

    // Delimiters
    SEMICOLON,      // ;
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    COMMA,          // ,

    // Special
    END_OF_FILE,
    UNKNOWN
};

// ==================== Token Struct ====================
struct Token {
    TokenType type;
    std::string value;
    int line;

    Token(TokenType t, const std::string& v, int l)
        : type(t), value(v), line(l) {}

    Token() : type(TokenType::UNKNOWN), value(""), line(0) {}
};

// ==================== Utility Function ====================
// Converts a TokenType enum value to a readable string (for debugging)
inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::INTEGER_LITERAL: return "INTEGER_LITERAL";
        case TokenType::FLOAT_LITERAL:   return "FLOAT_LITERAL";
        case TokenType::STRING_LITERAL:  return "STRING_LITERAL";
        case TokenType::IDENTIFIER:      return "IDENTIFIER";
        case TokenType::KW_INT:          return "KW_INT";
        case TokenType::KW_FLOAT:        return "KW_FLOAT";
        case TokenType::KW_STRING:       return "KW_STRING";
        case TokenType::KW_IF:           return "KW_IF";
        case TokenType::KW_ELSE:         return "KW_ELSE";
        case TokenType::KW_WHILE:        return "KW_WHILE";
        case TokenType::KW_PRINT:        return "KW_PRINT";
        case TokenType::KW_INPUT:        return "KW_INPUT";
        case TokenType::PLUS:            return "PLUS";
        case TokenType::MINUS:           return "MINUS";
        case TokenType::MULTIPLY:        return "MULTIPLY";
        case TokenType::DIVIDE:          return "DIVIDE";
        case TokenType::ASSIGN:          return "ASSIGN";
        case TokenType::EQUAL:           return "EQUAL";
        case TokenType::NOT_EQUAL:       return "NOT_EQUAL";
        case TokenType::LESS:            return "LESS";
        case TokenType::GREATER:         return "GREATER";
        case TokenType::LESS_EQ:         return "LESS_EQ";
        case TokenType::GREATER_EQ:      return "GREATER_EQ";
        case TokenType::AND:             return "AND";
        case TokenType::OR:              return "OR";
        case TokenType::NOT:             return "NOT";
        case TokenType::SEMICOLON:       return "SEMICOLON";
        case TokenType::LPAREN:          return "LPAREN";
        case TokenType::RPAREN:          return "RPAREN";
        case TokenType::LBRACE:          return "LBRACE";
        case TokenType::RBRACE:          return "RBRACE";
        case TokenType::COMMA:           return "COMMA";
        case TokenType::END_OF_FILE:     return "END_OF_FILE";
        case TokenType::UNKNOWN:         return "UNKNOWN";
        default:                         return "UNKNOWN";
    }
}

#endif // TOKEN_H
