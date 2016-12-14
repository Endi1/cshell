#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>



/* 
   Function declarations for builtin shell commands:
*/

int csh_cd(char **args);
int csh_help(char **args);
int csh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions
*/

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &csh_cd,
  &csh_help,
  &csh_exit
};

int csh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations
*/

int csh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "cshell: expected argument to \"cd\"");
  } else {
    if(chdir(args[1]) != 0) {
      perror("cshell");
    }
  }
  return 1;
}

int csh_help(char **args) {
  int i;
  printf("Endi Sukaj's CShell\n");
  printf("Type program names and arguments and hit enter\n");
  printf("The following are built in:\n");

  for (i = 0; i < csh_num_builtins(); i++) {
    printf(" %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs\n");
  return 1;
}

int csh_exit(char **args) {
  return 0;
}

int csh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("cshell");
    }
    exit(0);
  } else if (pid < 0) {
    // Error forking
    perror("cshell");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int csh_execute (char **args) {
  int i;

  if (args[0] == NULL) {
    // Empty command
    return 1;
  }

  for(i = 0; i < csh_num_builtins(); i++) {
    if(strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return csh_launch(args);
}

#define CSH_TOK_BUFSIZE 64
#define CSH_TOK_DELIM " \t\r\n\a"
char **csh_split_line(char *line) {
  int bufsize = CSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(sizeof(char) * bufsize);
  char *token;

  if (!tokens) {
    fprintf(stderr, "cshell: allocation error\n");
    exit(0);
  }

  token = strtok(line, CSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += CSH_TOK_BUFSIZE;
      tokens = realloc(tokens, sizeof(char) * bufsize);
      if(!tokens) {
        fprintf(stderr, "cshell: allocation error\n");
        exit(0);
      }
    }

    token = strtok(NULL, CSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

char *csh_read_line(void) {
  char *lineptr = NULL;
  size_t bufsize = 0;
  getline(&lineptr, &bufsize, stdin);
  return lineptr;
}

void cshell_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = csh_read_line();
    args = csh_split_line(line);
    status = csh_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  // Load config files, if any.

  // Run command loop.
  cshell_loop();

  // Perform any shutdown/cleanup

  return 0;
}
