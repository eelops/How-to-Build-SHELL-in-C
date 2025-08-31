#include <sys/wait.h>
//waitpid()
#include <unistd.h>
//chdir(),fork(),exec(),pid_t
#include <stdlib.h>
//malloc(),realloc(),free(),exit(),execvp(),EXIT_SCUEESS,EXIT_FAILURE
#include <stdio.h>
//fprintf(),printf(),stderr,getchar(),perror()
#include <string.h>
//strcmp(),strtok()

#define ESH_TOK_BUFSIZE 64
#define ESH_TOK_DELIM " \t\n\a\r"
#define ESH_BUFSIZE 1024

//内置函数计数函数
int num_builtin_func(void);
//内置函数声明处
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

//Shell实现函数声明处
int esh_loop(void);
char *esh_line(void);
char **esh_split(char *line);
int esh_launch(char **args);
int esh_execute(char **args);

int esh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("->");
        line = esh_line();
        args = esh_split(line);
        status = esh_execute(args);

        free(line);
        free(args);
    } while (status);
}

char *esh_line(void)
{
    int c;
    int bufsize = ESH_BUFSIZE;
    int position = 0;

    char *buffer = malloc(sizeof(char) * bufsize);

    if (!buffer) {
        fprintf(stderr, "esh: allocation error.\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if (position >= ESH_BUFSIZE) {
            bufsize += ESH_BUFSIZE;
            buffer = realloc(buffer, bufsize);

            if (!buffer) {
                fprintf(stderr, "esh: allocation error.\n");
                exit(EXIT_FAILURE);
            }
        }

        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }

        position++;
    }

    
}

char **esh_split(char *line)
{
    int bufsize = ESH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;

    if (!tokens) {
        fprintf(stderr, "esh: allocation error.\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, ESH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += ESH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if (!tokens) {
                fprintf(stderr, "esh: allocation error.\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, ESH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int esh_launch(char **args)
{
    pid_t pid,wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // 子进程
        if (execvp(args[0], args) == -1) {
            perror("esh");
        } 
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // 错误分支
        perror("esh");
    } else {
        // 父进程
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("waitpid failed:");
                break;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/*
builtin func in Shell
*/


char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[])(char **) = {
    &shell_cd,
    &shell_help,
    &shell_exit
};

int num_builtin_func(void) 
{
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "esh: expected arguments to \"cd\".\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("esh");
        }
    }

    return 1;
}

int shell_help(char **args)
{
    int i;
    printf("EELOP's esh:\n");   
    printf("The following are built in:\n");

    for (i = 0; i < num_builtin_func(); i++) {
        printf("\t%s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");

    return 1;
}

int shell_exit(char **args)
{
    return 0;
}

int esh_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < num_builtin_func(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return esh_launch(args);
}

int main(void) {
    esh_loop();

    return EXIT_SUCCESS;
}