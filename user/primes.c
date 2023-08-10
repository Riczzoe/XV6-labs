#include "kernel/types.h"
#include "user/user.h"

#define PW  1   /* The index of the pipe for writing in the array */
#define PR  0   /* The index of the pipe for reading in the array */
#define INT_LEN sizeof(int)     /* The byte length of an integer  */

void determinePrime(int pr);

int main() {
    int i;
    int p[2];
    
    pipe(p);

    for (i = 2; i <= 35; i++) {
        write(p[PW], &i, INT_LEN);
    }

    if (fork() == 0) {
        close(p[PW]);
        determinePrime(p[PR]);
    }
    
    close(p[PR]);
    close(p[PW]);
    wait(0);

    exit(0);
}


void determinePrime(int pr) {
    int n, number, pid;
    int pdfs[2];

    if (!read(pr, &n, INT_LEN)) {
        close(pr);
        return;
    }

    printf("prime %d\n", n);
    pipe(pdfs);
    
    pid = fork();
    
    if (pid == 0) {
        close(pdfs[PW]);
        determinePrime(pdfs[PR]);
    } else {
        close(pdfs[PR]);
        while (read(pr, &number, INT_LEN)) {
            if (number % n != 0)
                write(pdfs[PW], &number, INT_LEN);
        }
        close(pdfs[PW]);

        wait(0);
    }

    close(pr);
}





