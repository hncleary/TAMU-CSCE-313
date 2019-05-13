#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <wait.h>

#include "Tokenizer.h"

using namespace std;

class Command
{
  public:
    string Name;
    vector<Token> Arguments;
    Command *Next;

    bool IsBackground = false;
    char RedirectDirection;
    string RedirectFilename;
};

class Shell
{
  private:
    Command *split(vector<Token> tokens)
    {
        Command *command = new Command();
        Command *current = command;

        for (Token token : tokens)
        {
            switch (token.Type)
            {
            case TokenType::CommandName:
            {
                current->Name = token.Value;
                break;
            }
            case TokenType::Argument:
            {
                current->Arguments.push_back(token);
                break;
            }
            case TokenType::Background:
            {
                current->IsBackground = true;
                break;
            }
            case TokenType::Pipe:
            {
                current->Next = new Command();
                current = current->Next;
                break;
            }
            case TokenType::Redirect:
            {
                current->RedirectDirection = token.Value.c_str()[0];
                break;
            }
            case TokenType::Filename:
            {
                current->RedirectFilename = token.Value;
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
    void Execute(vector<Token> tokens)
    {
        Command *command = split(tokens);

        int prevFD[2];
        int nextFD[2];
        bool isFirst = true;
        int status;

        pipe(prevFD);
        while (command)
        {
            cout << "\'" << command->RedirectFilename << "\'" << endl;
            // Special cases
            if (command->Name == "exit")
            {
                exit(0);
            }
            if (command->Name == "cd")
            {
                chdir(command->Arguments[0].Value.c_str());
                break;
            }
            if (command->Name == "jobs")
            {
                for (string process : processes)
                    cout << process << endl;
                break;
            }

            // Creating arguments for execvp
            const char **args = new const char *[command->Arguments.size() + 2];
            args[0] = command->Name.c_str();

            for (int i = 0; i < command->Arguments.size(); i++)
            {
                args[i + 1] = command->Arguments[i].Value.c_str();
            }
            args[command->Arguments.size() + 1] = NULL;


            // IO Redirection
            int redirectionFD;
            if (command->RedirectFilename.empty() == false)
            {
                int pid = fork();
                if (!pid)
                {
                    // Into file
                    if (command->RedirectDirection == '>')
                    {
                        redirectionFD = open(command->RedirectFilename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
                        dup2(redirectionFD, 1);
                    }
                    // From file
                    else
                    {
                        redirectionFD = open(command->RedirectFilename.c_str(), O_RDONLY | S_IRUSR);
                        dup2(redirectionFD, 0);
                    }
                    execvp(args[0], (char **)args);
                }
                else
                {
                    waitpid(pid, 0, 0);
                    close(redirectionFD);
                    command = command->Next;
                    break;
                }
            }

            // Actual piping
            if (command->Next)
            {
                pipe(nextFD);
            }
            int pid = fork();
            if (!pid)
            {
                // Child
                if(!isFirst) {
                    // Setting the previous pipe back to stdin
                    dup2(prevFD[0], 0);

                    // Removing old pipe
                    close(prevFD[1]);
                }
                if(command->Next) {
                    // Remove in from parent
                    close(nextFD[0]);
                    // Set stdout to current
                    dup2(nextFD[1], 1);
                    // Remove out to parent
                }
                // Doin the executin
                execvp(args[0], (char **)args);
            }
            else
            {
                // Parent
                if(!isFirst) {
                    // Close
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
            command = command->Next;
        }
    }
};
