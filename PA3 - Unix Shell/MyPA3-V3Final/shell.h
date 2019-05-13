#pragma once

#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <wait.h>
#include <string>
#include <regex>
#include <algorithm>


using namespace std;


enum inputType
{
    piping,
    redirecting,
    background,
    commanding,
    argument,
    file
};

struct Token
{
    inputType type;
    string value;
    bool typeString;
    Token(string value = "", inputType type = inputType::argument)
    : value(value), type(type), typeString(false) {}
};

class tokenizer
{
    static string trim(string s) {
        string ret = s;

        if(isspace(ret[ret.size() - 1])) ret = ret.substr(0, ret.size() - 1);
        if(isspace(ret[0])) ret = ret.substr(1, ret.size());

        return ret;
    }
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }
    static inline void trimTot(std::string &s) {
        ltrim(s);
        rtrim(s);
    }
   static inline std::string ltrim_copy(std::string s) {
       ltrim(s);
       return s;
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

        inputType lastToken = inputType::piping;
        bool openString = false;
        char quoteUsed = '\0';

        string str = "";
        while (ss >> token)
        {
            Token t(token);

            if (regex_match(t.value, pipe)){
                t.type = inputType::piping;
              }
            else if (regex_match(t.value, redirect)){
                t.type = inputType::redirecting;
              }
            else if (regex_match(t.value, background)){
                t.type = inputType::background;
              }
            else if (lastToken == inputType::piping){
                t.type = inputType::commanding;
              }
            else if (lastToken == inputType::redirecting){
                t.type = inputType::file;
              }
            else{
              t.type = inputType::argument;
            }

            bool isSingleQuoted = t.value.find('\'') != string::npos;
            bool isQuoted = isSingleQuoted || t.value.find('\"') != string::npos;
            bool isWithoutSpaces = regex_match(t.value, bothQuoted) && !openString;

            if(isQuoted || openString) {
                str += t.value;
                char thisQuote;
                if(isSingleQuoted){
                  thisQuote = '\'';
                }
                else{
                  thisQuote = '\"';
                }

                if(isQuoted && quoteUsed == thisQuote) {
                    // End of a string
                    openString = false;
                    t.value = str;
                    t.typeString = true;
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

            lastToken = t.type;

            intermediate.push_back(t);
        }

        // Because there might be no spaces :(
        for(auto token: intermediate)
        {
            if(token.typeString) {
                char apos = token.value[0];
                token.value.erase(remove(token.value.begin(), token.value.end(), apos), token.value.end());
            }
            if(regex_match(token.value, regex(".*[<>].*"))) {
                string temp = token.value;
                int found;

                if (temp.find('<') == string::npos){
                   found = temp.find('>');
                }
                else{
                  found = temp.find('<');
                }

                token.value = temp.substr(0, found);
                Token redirector(temp.substr(found, 1), inputType::redirecting);
                Token rest(temp.substr(found + 1, temp.size()), inputType::file);

                token.value = trim(token.value);
                redirector.value = trim(redirector.value);
                rest.value = trim(rest.value);


                if(!token.value.empty() ) {
                  ret.push_back(token);
                }
                if(! redirector.value.empty()){
                   ret.push_back(redirector);
                 }
                if(! rest.value.empty()) {
                  ret.push_back(rest);
                }
                continue;
            }
            ret.push_back(token);
        }

        return ret;
    }
    /*
    const string* which_starts(const std::string& string, const vecstr& tests,const std::string::size_type& pos) {
        for (int i = 0; i < tests.size(); i++) {
            if (starts_at(string, tests[i], pos)) {
                return &tests.at(i);
            }
        }

        return NULL;
    }

    const pairstr* which_starts(const std::string& string,const std::vector<pairstr>& tests,const std::string::size_type& pos) {
        for (int i = 0; i < tests.size(); i++) {
            if (starts_at(string, tests[i].first, pos)) {
                return &tests.at(i);
            }
        }
        return NULL;
    }
    */
};
class tokenOperation
{
  public:
    string name;
    vector<Token> parameters;
    tokenOperation *next;

    bool backgroundProcess = false;
    char redirectDir;
    string redirectFile;
};

class Shell
{
  private:
    tokenOperation *split(vector<Token> tokens)
    {
        tokenOperation *command = new tokenOperation();
        tokenOperation *current = command;

        for (Token token : tokens)
        {
            switch (token.type){
            case inputType::commanding:{
                current->name = token.value;
                break;
            }
            case inputType::argument:{
                current->parameters.push_back(token);
                break;
            }
            case inputType::background:{
                current->backgroundProcess = true;
                break;
            }
            case inputType::piping:{
                current->next = new tokenOperation();
                current = current->next;
                break;
            }
            case inputType::redirecting:{
                current->redirectDir = token.value.c_str()[0];
                break;
            }
            case inputType::file:{
                current->redirectFile = token.value;
                break;
            }
            default:
                break;
            }
        }
        return command;
    }

    vector<string> processes;
    
  public:
    void runCommand(vector<Token> tokens)
    {
        tokenOperation *command = split(tokens);

        int prevFD[2];
        int nextFD[2];
        bool isFirst = true;
        int status;

        pipe(prevFD);
        while (command)
        {
            cout << "\'" << command->redirectFile << "\'" << endl;
            if (command->name == "exit"){
                exit(0);
            }
            if (command->name == "cd"){
                chdir(command->parameters[0].value.c_str());
                break;
            }
            if (command->name == "jobs"){
                for (string process : processes)
                    cout << process << endl;
                break;
            }
            const char **args = new const char *[command->parameters.size() + 2];
            args[0] = command->name.c_str();

            for (int i = 0; i < command->parameters.size(); i++){
                args[i + 1] = command->parameters[i].value.c_str();
            }
            args[command->parameters.size() + 1] = NULL;
            // redirection
            int redirectionFD;
            if (! command->redirectFile.empty()){
                int pid = fork();
                if (!pid){
                    // redirect to a file "">""
                    if (command->redirectDir == '>'){
                        redirectionFD = open(command->redirectFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
                        dup2(redirectionFD, 1);
                    }
                    // redirect from a file ""<""
                    else{
                        redirectionFD = open(command->redirectFile.c_str(), O_RDONLY | S_IRUSR);
                        dup2(redirectionFD, 0);
                    }
                    //execution
                    execvp(args[0], (char **)args);
                }
                else{
                    waitpid(pid, 0, 0);
                    close(redirectionFD);
                    command = command->next;
                    break;
                }
            } //piping
            if (command->next){
                pipe(nextFD);
            }
            int pid = fork();
            if (!pid){
                // Child
                if(!isFirst) {
                    //restore previous pipe, close new pipe
                    dup2(prevFD[0], 0);
                    close(prevFD[1]);
                }
                if(command->next) {
                    // remove in
                    close(nextFD[0]);
                    dup2(nextFD[1], 1);
                    // remove out
                }
                execvp(args[0], (char **)args);
            }
            else{
            //parent process
                if(!isFirst) {
                    close(prevFD[0]);
                    close(prevFD[1]);
                }
                if(command->next) {
                    prevFD[0] = nextFD[0];
                    prevFD[1] = nextFD[1];
                }
            }
            waitpid(pid, 0, 0);
            isFirst = false;
            command = command->next;
        }
    }
};
