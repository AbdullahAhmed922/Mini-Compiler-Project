#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "token.h"

class Lexer {
public:
    // Constructor: takes the raw source code as a string
    Lexer(const std::string& sourceCode);

    // Main method: scans the source and returns all tokens
    std::vector<Token> tokenize();

    // Check if any lexical errors occurred
    bool hasErrors() const;

    // Get the list of error messages
    std::vector<std::string> getErrors() const;

private:
    std::string source;             // the source code being scanned
    int pos;                        // current position in source
    int line;                       // current line number
    std::vector<std::string> errors;

    // Helper methods
    char currentChar();             // returns current character
    char peek();                    // returns next character without advancing
    void advance();                 // move to next character
    void skipWhitespace();          // skip spaces, tabs, newlines
    void skipComment();             // skip single-line comments (//)
    Token readNumber();             // read integer or float literal
    Token readString();             // read string literal ("...")
    Token readIdentifierOrKeyword();// read identifier or keyword
    void addError(const std::string& msg);
};

#endif // LEXER_H
