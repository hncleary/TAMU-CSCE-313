#pragma once

#include <string>
#include <sstream>
#include <regex>
#include <vector>
#include <algorithm>

using namespace std;

enum TokenType
{
    Pipe,
    Redirect,
    Background,
    CommandName,
    Argument,
    Filename
};

struct Token
{
    TokenType Type;
    string Value;
    bool IsString;
    Token(string value = "", TokenType type = TokenType::Argument)
    : Value(value), Type(type), IsString(false) {}
};

class Tokenizer
{
    static string trim(string s) {
        string ret = s;

        if(isspace(ret[ret.size() - 1])) ret = ret.substr(0, ret.size() - 1);
        if(isspace(ret[0])) ret = ret.substr(1, ret.size());

        return ret;
    }
  public:
    static vector<Token> Tokenize(const string &s)
    {
        vector<Token> ret;
        vector<Token> intermediate;
        stringstream ss(s);
        string token;

        regex pipe("^(\\|)$");
        regex redirect("[<>]");
        regex background("^(&)$");
        regex singleQuoted("\'");
        regex doubleQuoted("(\".+)|(.+\")");
        regex bothQuoted("(\".*\"|\'.*\')");

        TokenType lastToken = TokenType::Pipe;
        bool openString = false;
        char quoteUsed = '\0';

        string str = "";
        while (ss >> token)
        {
            Token t(token);

            if (regex_match(t.Value, pipe))
                t.Type = TokenType::Pipe;
            else if (regex_match(t.Value, redirect))
                t.Type = TokenType::Redirect;
            else if (regex_match(t.Value, background))
                t.Type = TokenType::Background;
            else if (lastToken == TokenType::Pipe)
                t.Type = TokenType::CommandName;
            else if (lastToken == TokenType::Redirect)
                t.Type = TokenType::Filename;
            else
                t.Type = TokenType::Argument;

            bool isSingleQuoted = t.Value.find('\'') != string::npos;
            bool isQuoted = isSingleQuoted || t.Value.find('\"') != string::npos;
            bool isWithoutSpaces = regex_match(t.Value, bothQuoted) && !openString;

            if(isQuoted || openString) {
                str += t.Value;
                char thisQuote = (isSingleQuoted) ? '\'' : '\"';

                if(isQuoted && quoteUsed == thisQuote) {
                    // End of a string
                    openString = false;
                    t.Value = str;
                    t.IsString = true;
                    str = "";
                }
                else if(isQuoted && openString == false) {
                    openString = true;
                    str += " ";
                    quoteUsed = thisQuote;
                    continue;
                }
                else {
                    str += " ";
                    continue;
                }
            }

            lastToken = t.Type;

            intermediate.push_back(t);
        }

        // Because there might be no spaces :(
        for(auto token: intermediate)
        {
            if(token.IsString) {
                char apos = token.Value[0];
                token.Value.erase(remove(token.Value.begin(), token.Value.end(), apos), token.Value.end());
            }
            if(regex_match(token.Value, regex(".*[<>].*"))) {
                string temp = token.Value;
                int found = (temp.find('<') == string::npos) ? temp.find('>') : temp.find('<');

                token.Value = temp.substr(0, found);
                Token redirector(temp.substr(found, 1), TokenType::Redirect);
                Token rest(temp.substr(found + 1, temp.size()), TokenType::Filename);

                token.Value = trim(token.Value);
                redirector.Value = trim(redirector.Value);
                rest.Value = trim(rest.Value);

                if(token.Value.empty() == false) ret.push_back(token);
                if(redirector.Value.empty() == false) ret.push_back(redirector);
                if(rest.Value.empty() == false) ret.push_back(rest);
                continue;
            }
            ret.push_back(token);
        }

        return ret;
    }
};
