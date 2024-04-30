#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#define HISTORY_SIZE 10
#define MAX_INPUT_LENGTH 100

// function to change directory (cd)
void cd(char *args[]) {
    chdir(args[1] ? args[1] : getenv("HOME"));
}

// function to print working directory (pwd)
void pwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("Error");
    }
}

// history functionality
char historyQueue[HISTORY_SIZE][MAX_INPUT_LENGTH];
int order = 1;

// function to keep history of commands
void keepHistory(char *args) {
    if (order <= HISTORY_SIZE) {
        strcpy(historyQueue[order - 1], args);
        order++;
    } else {
        for (int i = 1; i <= HISTORY_SIZE; i++) {
            if (i != HISTORY_SIZE) {
                strcpy(historyQueue[i - 1], historyQueue[i]);
            } else {
                break;
            }
        }
        strcpy(historyQueue[9], args);
    }
}

// function to display command history
void history() {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (strlen(historyQueue[i]) > 0) {
            printf("[%d] %s\n", (i + 1), historyQueue[i]);
        }
    }
}

// function to exit the shell
int exitS() {
    exit(0);
}

// signal handler for child processes
void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    signal(SIGCHLD, handle_sigchld);

    while (1) {
        char input[MAX_INPUT_LENGTH];
        char *args[10] = {NULL};
        char *args2[10] = {NULL};
        char *splitedInput;
        bool lock = true;
        bool isBackgroundProcess = false;
        bool isPipe = false;
        int index = 0;

        printf("myshell> ");
        fgets(input, MAX_INPUT_LENGTH, stdin);
        char *refreshedInput = strtok(input, "\n");

        // if input is empty, continue to next iteration
        if (strlen(input) == 1) {
            continue;
        }

        // remove newline character from input
        strcpy(input, refreshedInput);
        keepHistory(input);
        splitedInput = strtok(input, " ");

        // tokenize input and parse arguments
        while (splitedInput != NULL) {
            if (lock && (strcmp(splitedInput, "|") == 0 || strcmp(splitedInput, "&&") == 0)) {
                index = -1;
                lock = false;
                if (strcmp(splitedInput, "|") == 0) {
                    isPipe = true;
                }
            } else if (strcmp(splitedInput, "&") == 0) {
                isBackgroundProcess = true;
                splitedInput = NULL;
                break;
            } else {
                if (lock == true) {
                    args[index] = splitedInput;
                } else {
                    args2[index] = splitedInput;
                }
            }
            splitedInput = strtok(NULL, " ");
            index++;
        }

        if (strcmp(args[0], "cd") == 0) {
            cd(args);
        } else if (strcmp(input, "pwd") == 0) {
            pwd();
        } else if (strcmp(input, "history") == 0) {
            history();
        } else if (strcmp(input, "exit") == 0) {
            exitS();
        } else {
            if (lock == true) {
                // execute single command except cd, pwd, history, exit
                // fork a new process
                pid_t process = fork();
                if (process == 0) {
                    // child process

                    if (execvp(args[0], args) == -1) {
                        // execvp fails
                        perror("Error");
                    }
                    exit(EXIT_FAILURE);
                } else if (process < 0) {
                    // fork fails
                    perror("Error");
                    return 1;
                } else {
                    // parent process
                    
                    // if isBackgroundProcess is false, wait for the child process to finish
                    if (!isBackgroundProcess) {
                        wait(NULL);
                    } else {
                        // if isBackgroundProcess is true, print the process ID of the child process
                        printf("%d\n", process);
                    }
                }
            }
            else if (isPipe) {
                // execute piped commands (|)
            
                int pipefd[2];
                pid_t pid1, pid2;
                
                // create a pipe for communication between the two child processes
                if (pipe(pipefd) == -1) {
                    // pipe creation fails
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
                
                // fork the first child process
                pid1 = fork();
                if (pid1 < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid1 == 0) {
                    // child process 1
                    
                    // close the read end of the pipe
                    close(pipefd[0]);
                    
                    // redirect stdout to the write end of the pipe
                    dup2(pipefd[1], STDOUT_FILENO);
                    
                    // close the write end of the pipe
                    close(pipefd[1]);
                    
                    // execute the first command
                    execvp(args[0], args);
                    
                    // if execvp fails, report the error and exit
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
                
                // fork the second child process
                pid2 = fork();
                if (pid2 < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid2 == 0) {
                    // child process 2
                    
                    // close the write end of the pipe
                    close(pipefd[1]);
                    
                    // redirect stdin to the read end of the pipe
                    dup2(pipefd[0], STDIN_FILENO);
                    
                    // close the read end of the pipe
                    close(pipefd[0]);
                    
                    // execute the second command
                    execvp(args2[0], args2);
                    
                    // if execvp fails, report the error and exit
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
                
                // close both ends of the pipe in the parent process
                close(pipefd[0]);
                close(pipefd[1]);
                
                // if isBackgroundProcess is false, wait for both child processes to finish
                if (!isBackgroundProcess) {
                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);
                }
            }
            else {
                // execute chained commands with logical AND operator (&&)
                
                pid_t pid1, pid2;
                
                // fork the first child process
                pid1 = fork();

                if (pid1 < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid1 == 0) {
                    // first child process
                    
                    // execute the first command
                    execvp(args[0], args);
                    
                    // if execvp fails, report the error and exit
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }

                // parent process
                                
                // wait for the first process to finish
                int status;
                waitpid(pid1, &status, 0);
                
                // check if the child process exited successfully (status 0)
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    // fork the second child process
                    pid2 = fork();

                    if (pid2 < 0) {
                        perror("fork");
                        exit(EXIT_FAILURE);
                    } else if (pid2 == 0) {
                        // second child process
                        
                        // execute the second command
                        execvp(args2[0], args2);
                        
                        // if execvp fails, report the error and exit
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }
                    
                    // parent process
                    
                    // if isBackgroundProcess is false, wait for the second child process to finish
                    if (!isBackgroundProcess) {
                        waitpid(pid2, NULL, 0);
                    }
                }
            }
        }
    }
}
