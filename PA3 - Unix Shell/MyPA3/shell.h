#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fcntl.h>  // open
#include <unistd.h> // close
#include <vector>
#include <wait.h>

#include "token.h"

using namespace std;

class Command {
  public:
    string Name;
    vector<Token> Arguments;
    Command *Next;

    bool isBackgroundProcess = false;
    char redirectDirection;
    string redirectFile;
};

class shell {
private:
  Command *split(vector<Token> tokens){
    Command *command = new Command();
    Command *current = command;

    for (Token token : tokens){
      switch (token.type)
      {
        case kindOfToken::CommandName:{
          current->Name = token.value;
          break;
        }
        case kindOfToken::Argument:{
          current->Arguments.push_back(token);
          break;
        }
        case kindOfToken::Background:{
          current->isBackgroundProcess = true;
          break;
        }
        case kindOfToken::Pipe:{
          current->Next = new Command();
          current = current->Next;
          break;
        }
        case kindOfToken::Redirect:{
          current->redirectDirection = token.value.c_str()[0];
          break;
        }
        case kindOfToken::Filename:{
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
  void Execute(vector<Token> tokens){
    Command *command = split(tokens);
    int prevFD[2];
    int nextFD[2];
    bool isFirst = true;
    int status;

    pipe(prevFD);
    while (command){
      cout << "\'" << command->redirectFile << "\'" << endl;
      if(command->Name == "exit"){
        exit(0);
      }
      else if(command->Name == "cd"){
        chdir(command->Arguments[0].value.c_str());
        break;
      }
      else if(command->Name == "jobs"){
        for (string process : processes){
          cout << process << endl;
          break;
        }
      }

      const char **args = new const char *[command->Arguments.size() + 2];
      args[0] = command->Name.c_str();

      for (int i = 0; i < command->Arguments.size(); i++)
      {
        args[i + 1] = command->Arguments[i].value.c_str();
      }
      args[command->Arguments.size() + 1] = NULL;

      /*
      if(testing) {
        for(int i = 0; args[i] != NULL; i++){
          cout << args[i] << " " ;
        }
        cout << endl;
      }
      */
      //redirection
      int redirectionFD;
      if (command->redirectFile.empty() == false){
        int pid = fork();
        if(!pid){
          //into a file
          if (command->redirectDirection = '>'){
            redirectionFD = open(command->redirectFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
            dup2(redirectionFD, 1);
          }
          //from a file
          else{
            redirectionFD = open(command->redirectFile.c_str(), O_RDONLY | S_IRUSR);
            dup2(redirectionFD, 0);
          }
          execvp(args[0], (char **)args);
        }
        //is pid
        else{
          waitpid(pid, 0, 0);
          close(redirectionFD);
          command = command->Next;
          break;
        }
      }
      //piping
      if(command->Next){
        pipe(nextFD);
      }
      int pid = fork();
      if(!pid){
        //child
        if(!isFirst){
          //setting previous pipe to stdin
          dup2(prevFD[0],0);
          //destroying old Pipe
          close(prevFD[1]);
        }
        if(command->Next){
          //remove in from the parent process
          close(nextFD[0]);
          //set stdout ot current process
          dup2(nextFD[1], 1);
          //remove out from the parent process
        }
        //exectue the Command
        execvp(args[0], (char **)args);
      }
      else
      {
        //closing parent
        if(!isFirst) {
        close(prevFD[0]);
        close(prevFD[1]);
        }
        if(command->Next) {
          prevFD[0] = nextFD[0];
          prevFD[1] = nextFD[1];
        }
      }
      waitpid(pid, 0, 0);
      isFirst = false;
      command  = command->Next;
    }
  }
};
