#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CASH_RL_BUFFSIZE 1024
#define CASH_TOK_BUFFSIZE 64
#define CASH_TOK_DELIM " \t\r\n\a"

/**
 * Reads line from stdin
**/

char * cash_read_line(void)
{
    static char *line = NULL;
    ssize_t buffsize = 0; // getline will allocate
    getline(&line, &buffsize, stdin);
    return line;
}

/**
 * Splits line into tokens and returns array
 * of said tokens
**/

char ** cash_split_line(char *line)
{
    int buffsize = CASH_TOK_BUFFSIZE;
    int position = 0;
    char **tokens = malloc(buffsize * sizeof(char*));
    char *tok;

    if (!tokens) {
        fprintf(stderr, "ca$h: allocation error\n");
        exit(EXIT_FAILURE);
    }

    tok = strtok(line, CASH_TOK_DELIM); // get first token in string
    while (tok != NULL) {
        tokens[position] = tok;
        position++;

        if (position >= buffsize) {
            buffsize += CASH_TOK_BUFFSIZE;
            tokens = realloc(tokens, buffsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "ca$h: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        tok = strtok(NULL, CASH_TOK_DELIM); // get the rest
    }
    tokens[position] = NULL;
    return tokens;
}

/**
 * If command entered by user is not builtin,
 * then launch child process to execute
**/

int cash_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) { // child process
        if (execvp(args[0], args) == -1) {
            perror("ca$h");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("ca$h");
    } else { // parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


int cash_cd(char **args);
int cash_help(char **args);
int cash_exit(char **args);

/**
 * Builtin command setup
**/
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &cash_cd,
    &cash_help,
    &cash_exit
};

int cash_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/**
 * Builtin command implementations
**/

int cash_cd (char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "ca$h: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("ca$h");
        }
    }
    return 1;
}

int cash_help (char **args)
{
    int i;
    printf("Jese Camilo's CA$H Shell\n");
    printf("Type commands and hit ENTER.");
    printf("The following commands are available for use:\n");

    for (i = 0; i < cash_num_builtins(); i++) {
        printf("    %s\n", builtin_str[i]);
    }

    printf("Use the man command for information regarding other commands.\n");
    return 1;
}

int cash_exit (char **args)
{
    return 0;
}

/**
 * Execute command. Check if builtin, otherwise
 * launch process
**/

int cash_execute (char **args)
{
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < cash_num_builtins(); i ++) {
        if (strcmp(args[0], builtin_str[i]) == 0)
            return (*builtin_func[i])(args);
    }

    return cash_launch(args);
}

/**
 * Shell loop utilizing above functions
**/

void cash_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("ca$h> ");
        line = cash_read_line();
        args = cash_split_line(line);
        status = cash_execute(args);

        free(line);
        free(args);
    } while (status);
}

/**
 * Program entry
**/

int main(int argc, char **argv)
{
    cash_loop(); // run shell loop

    return EXIT_SUCCESS;
}
