# Shell-Like C Program
This shell-like program displays the command line prompt `myshell>` and waits for the user's command. It reads the user’s next command, parses it into separate tokens that are used to fill the argument vector for the command to be executed, and executes it. This shell-like program supports the following built-in commands:

* **cd <directory>:** changes the current directory to <directory>.
* **pwd:** prints the current working directory.
* **history:** prints the 10 most recently entered commands in this session, including this command.
* **exit:** terminates the shell process.
* For other shell commands, this shell-like program consider them as system commands and execute them with creating a child process with **fork()**.

  Try:

      ls -l -a

### Also this shell-like program handles other advanced processes:

* **Supports background processes:**  This program handles both foreground and background processes for system commands. When a process runs in the foreground, this program waits for the task to complete, then prompts the user for another command. A background process is indicated by placing an ampersand character (’&’) at the end of an input line. When a process runs in background, this program does not wait for the task to complete, but display the process id of the background process and immediately prompt the user for another command.

    Try:
  
        gedit &

* **Supports the pipe operator:** If the user types `ls -al | sort`, after the ls command run, the sort command runs with ls command's output coming from that pipe.

    Try:
  
        ls -l | wc -l

* **Supports the logical AND operator:** This program handles conditionally chained processes using the logical AND operator (‘&&’). Second command must run if and only if the first one is successful. This behavior is the result of **short-circuit evaluation**.

    Try:
  
        gcc main.c && ./a.out

## To run this shell-like program:
1. Download `main.c`.
2. Open a command prompt, ensuring it is launched within a *Bash* shell.
3. Compile `main.c` using `gcc main.c -o shell`.
4. Execute the compiled program by running `./shell`.
5. Enter your command.
