#pragma once
#include <string>
#include <vector>
#include <map>

using namespace std;

enum class TokenType {
    OPEN_TAG,
    CLOSE_TAG,
    SELF_CLOSING,
    TEXT,
    COMMENT,
    DOCTYPE
};

struct Token {
    TokenType type;
    string tagName;
    map<string, string> attributes;
    string text;
};

vector<Token> tokenize(const string& html);