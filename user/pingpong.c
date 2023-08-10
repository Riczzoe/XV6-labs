#include "kernel/types.h"
#include "user/user.h"

#define _MAX_LEN_ 10    /* max length of buffer */

int main() {
    int p2c[2];     /* pipe from parent to child */
    int c2p[2];     /* pipe from child to parent */
    char byte4Send = 'c';
    char buf[_MAX_LEN_];
    
    pipe(p2c);
    pipe(c2p);

    if (fork() == 0) {
        close(p2c[1]);
        close(c2p[0]);

        read(p2c[0], buf, sizeof(buf));
        printf("%d: received ping\n", getpid());
        write(c2p[1], &byte4Send, 1);

        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    } else {
        close(p2c[0]);
        close(c2p[1]);

        write(p2c[1], &byte4Send, 1);
        read(c2p[0], buf, sizeof(buf));
        printf("%d: received pong\n", getpid());

        close(p2c[1]);
        close(c2p[0]);
    }

    exit(0);
}
