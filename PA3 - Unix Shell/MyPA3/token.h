#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <regex>
#include <vector>

using namespace std;

enum kindOfToken {
  Pipe,
  Redirect,
  Background,
  CommandName,
  Argument,
  Filename
};

struct Token{
  kindOfToken type;
  string value;
  bool typeString;

  Token (string value = "", kindOfToken kind = kindOfToken::Argument )
  : value(value),type(type),typeString(false){}
};

class tokenizer{
  static string trim(string s){
    string returnString = s;
    //removes blank space at the end of a string
    if(isspace(returnString[returnString.size() - 1])){
      returnString = returnString.substr(0,returnString.size() - 1);
    }
    //removes blank space a the beginning of a string
    if(isspace(returnString[0])){
      returnString = returnString.substr(1,returnString.size() );
    }
    return returnString;
  }
public:
  static vector<Token> tokenize(const string &s){

    vector<Token> returnVector;
    vector<Token> tempVector;
    stringstream ss(s);
    string token;

    regex pipe("^(\\|)$");
    regex redirect("[<>]");
    regex background("^(&)$");
    regex singleQuoted("\'");
    regex doubleQuoted("(\".+)|(.+\")");
    regex bothQuoted("(\".*\"|\'.*\')");

    kindOfToken lastToken = kindOfToken::Pipe;
    bool openString = false;
    char delimiter = '\0';

    string str = "";
    while (ss >> token){
      Token t(token);

      if(regex_match(t.value, pipe)){
        t.type = kindOfToken::Pipe;
      }
      else if(regex_match(t.value, redirect)){
        t.type = kindOfToken::Redirect;
      }
      else if(regex_match(t.value, background)){
        t.type = kindOfToken::Background;
      }
      else if(lastToken == kindOfToken::Pipe){
        t.type = kindOfToken::CommandName;
      }
      else if(lastToken == kindOfToken::Redirect){
        t.type = kindOfToken::Filename;
      }
      else{
        t.type = kindOfToken::Argument;
      }

      bool isSingleQuoted = t.value.find('\'') != string::npos;
      bool isQuoted = isSingleQuoted || t.value.find('\"') != string::npos;
      bool isWithoutSpaces = regex_match(t.value,bothQuoted) &&!openString;

      if(isQuoted || openString) {
        str += t.value;
        //if single quoted \' else \"
        char thisQuote = (isSingleQuoted) ? '\'' : '\"';

        if(isQuoted && delimiter == thisQuote) {
          //end of a string
          openString = false;
          t.value = str;
          t.typeString = true;
          str = "";
        }
        else if (isQuoted && openString == false) {
          openString = true;
          str += "";
          delimiter = thisQuote;
          continue;
        }
        else {
          str += "";
          continue;
        }
      }
      lastToken = t.type;

      tempVector.push_back(t);
    }
    for (auto token: tempVector)
    {
      if(token.typeString) {
        char apos = token.value[0];
        token.value.erase(remove(token.value.begin(), token.value.end(), apos), token.value.end());
      }
      if(regex_match(token.value, regex(".*[<>].*"))){
        string temp = token.value;
        int found = (temp.find('<') == string::npos) ? temp.find('>') : temp.find('<');

        token.value = temp.substr(0,found);
        Token redirector(temp.substr(found, 1), kindOfToken::Redirect);
        Token rest(temp.substr(found + 1, temp.size()), kindOfToken::Filename);

        //remove space from beginning and end
        token.value = trim(token.value);
        redirector.value = trim(redirector.value);
        rest.value = trim(rest.value);

        if(token.value.empty() == false) returnVector.push_back(token);
        if(redirector.value.empty() == false) returnVector.push_back(redirector);
        if(rest.value.empty() == false) returnVector.push_back(rest);
        continue;
      }
      returnVector.push_back(token);
    }
    return returnVector;
  }
};
