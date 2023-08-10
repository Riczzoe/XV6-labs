#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

#define _MAX_LEN_ 512

void free_argv(char **args, int len) {
    int i;
    for (i = 0; i < len; i ++) {
        free(args[i]);
    }
    free(args);
}

int read_argv(char **argvs, int len) {
    char buf[_MAX_LEN_], *p, line[_MAX_LEN_];
    int i = 0, readSize, j;

    p = line;
    while ((readSize = read(0, buf, _MAX_LEN_)) > 0 && i < MAXARG) {
        for (j = 0; j < readSize; j ++) {
            *(p++) = buf[j];
            if (buf[j] == '\n') {
                *(--p) = 0;
                argvs[i] = (char *) malloc(strlen(line) * sizeof(char));
                strcpy(argvs[i], line);
                p = line;
                i++;
            } 
        }
    }

    if (i == MAXARG) {
        fprintf(2, "xargs: too many arguments\n");
        free_argv(argvs, i);
        exit(1);
    }
    argvs[i] = 0;

    return i;
}


/*
 * void print_argv(char **argvs, int len) {
 *     int i;
 *     for (i = 0; i < len; i ++) {
 *         printf("argvs[%d]: %s\n", i, argvs[i]);
 *     }
 *     printf("\n-----------------------------------\n");
 * }
 */

int main(int argc, char *args[]) {
    char **allXargArgs = (char **) malloc(MAXARG * sizeof(char *));
    char *command;
    int i;

    command = args[1];
    /* printf("command: %s\n", command); */
    int xargArgc = read_argv(allXargArgs, MAXARG);
    /* print_argv(allXargArgs, xargArgc); */

    char **argv = (char **) malloc((argc + 1) * sizeof(char *));
    for (i = 0; i < argc - 1; i++) {
        argv[i] = args[i + 1];
    }
    argv[argc] = 0;
    for (i = 0; i < xargArgc; i ++) {
        argv[argc - 1] = allXargArgs[i];
        if (fork() == 0) {
            exec(command, argv);
            fprintf(2, "xargs: exec %s failed\n", command);
            exit(1);
        }
        wait(0);
    }

    /*
     * for (i = 1; i < xargArgc; i ++) {
     *     wait(0);
     * }
     */

    free_argv(allXargArgs, xargArgc);
    return 0;
}

