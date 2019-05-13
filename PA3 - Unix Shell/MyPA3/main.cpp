#include "unistd.h"
#include <string>
#include <limits.h>
#include <pwd.h>
#include <iostream>

#include "token.h"
#include "shell.h"

using namespace std;

bool prompt(bool isSilent){
  char cwd[PATH_MAX];
  cout << getpwuid(geteuid())->pw_name << ":" << getcwd(cwd, sizeof(cwd)) << "$ ";
  return true;
}

int main(int argc, char **argv){
  cout << "is this even working?" << endl;
  int argDelimiter;
  bool testingFlag = false;
  while((argDelimiter = getopt(argc, argv, "t")) != -1){
    switch (argDelimiter)
    {
      case 't':{
        testingFlag = true;
        break;
      }
      case '?':{
        exit(1);
        break;
      }
      default:{
        break;
      }
    }
    string input;
    char cwd[PATH_MAX];

    shell shell1;
    if(testingFlag)
        cout << endl << "------------------- Testing --------------------" << endl << endl;


    prompt(testingFlag);
    while(getline(cin, input)){
      vector<Token> tokens = tokenizer::tokenize(input);

      if(testingFlag) {
          for(auto t: tokens)
              cout << "\'" << t.value << "\' " << t.type << ' ';
          cout << endl;
      }

      shell1.Execute(tokens);
      prompt(testingFlag);
    }
  }
}
