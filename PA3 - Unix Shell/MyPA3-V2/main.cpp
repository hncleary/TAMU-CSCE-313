#include "unistd.h"
#include <string>
#include <limits.h>
#include <pwd.h>
#include <iostream>

#include "Tokenizer.h"
#include "Shell.h"

using namespace std;

int main(int argc, char **argv) {
    int argDelimiter;


    string input;
    char cwd[PATH_MAX];
    Shell shell;

    cout << getpwuid(geteuid())->pw_name << ":" << getcwd(cwd, sizeof(cwd)) << "$ ";

    while(getline(cin, input)) {
        vector<Token> tokens = Tokenizer::Tokenize(input);
        shell.Execute(tokens);
        cout << getpwuid(geteuid())->pw_name << ":" << getcwd(cwd, sizeof(cwd)) << "$ ";

    }
}
