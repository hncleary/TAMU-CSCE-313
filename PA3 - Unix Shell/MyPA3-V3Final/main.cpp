#include "unistd.h"
#include <string>
#include <limits.h>
#include <pwd.h>
#include <iostream>

#include "shell.h"

using namespace std;

int main(int argc, char **argv) {

    string input;
    char cwd[PATH_MAX];
    Shell shell;

    cout << getpwuid(geteuid())->pw_name << ":~" << getcwd(cwd, sizeof(cwd)) << "$ ";

    while(getline(cin, input)) {
        vector<Token> tokens = tokenizer::Tokenize(input);
        shell.runCommand(tokens);
        cout << getpwuid(geteuid())->pw_name << ":~" << getcwd(cwd, sizeof(cwd)) << "$ ";

    }
}
