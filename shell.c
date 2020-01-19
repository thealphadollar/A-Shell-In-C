#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SHELL_BUFSIZE 1024
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

int execute(char **args);
char ** shell_split_line(char * line);
char* shell_read_line();
void shell_loop();
int shell_cd(char **args);
int shell_exit(char **args);
int shell_help(char **args);

char *built_in[] = 
{
    "cd",
    "exit",
    "help"
};

int (*built_in_func[]) (char **) = 
{
    &shell_cd,
    &shell_exit,
    &shell_help
};

int shell_num_builtins()
{
    return sizeof(built_in) / sizeof(char *);
}

int main(int argc, char **argv)
{
    // Setting for loading config files

    // Command loop
    shell_loop();

    // shutdown actions
    return EXIT_SUCCESS;
}

void shell_loop()
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("--> ");
        line = shell_read_line();
        args = shell_split_line(line);
        status = execute(args);

        // memory cleaning
        free(line);
        free(args);
    } while (status);

}

char * memory_failed_error()
{
    fprintf(stderr, "Shell: failed to allocate memory for line\n");
    exit(EXIT_FAILURE);
}

char* shell_read_line()
{
    int bufsize = SHELL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        memory_failed_error();
    }

    while(1)
    {
        c = getchar();

        if (c==EOF || c=='\n')
        {
            buffer[position] == '\0';
            return buffer;
        } else
        {
            buffer[position] = c;
        }
        position++;
        
        // handle buffer limit extension
        if (position >= bufsize)
        {
            bufsize += SHELL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer){
                memory_failed_error();
            }
        }
    }
}

char** shell_split_line(char* line)
{
    int bufsize = SHELL_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;

    if (!tokens) 
    {
        memory_failed_error();
    }

    token = strtok(line, SHELL_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += SHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                memory_failed_error();
            }
        }
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int shell_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) 
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0)
    {
        perror("forking error!");
    } else
    {
        // Parent process
        do 
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int execute(char **args)
{
    if (args[0] == NULL)
    {
        return 1;
    } else
    {
        for (int i = 0; i<shell_num_builtins(); i++)
        {
            if (strcmp(built_in[i], args[0]) == 0)
            {
                return (*built_in_func[i])(args);
            }
        }
        return shell_launch(args);
    }
    
}

int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "shell: expected argument to \"cd\" command!");
    } else
    {
        if (chdir(args[1]) != 0)
        {
            perror("shell");
        }
    }
    return 1;
}

int shell_help(char **args)
{
    printf("Shell help!\n");
    printf("The builtin commands are: \n");
    for (int i = 0; i<shell_num_builtins(); i++)
    {
        printf(" %s\n", built_in[i]);
    }

    printf("Use the man command for knowing about other commands!");
}

int shell_exit(char **args)
{
    return 0;
}