#include "kernel/types.h"
#include "user.h"

int main(int argc, char* argv[]) {
    int p0[2], p1[2];
    pipe(p0); pipe(p1);
    if (fork() == 0) { // kid
        char buffer[5] = {};
        close(p0[1]);
        close(p1[0]);

        read(p0[0], buffer, 4);
        printf("%d: received %s\n", getpid(), buffer);

        write(p1[1], "pong", 4);

        close(p0[0]);
        close(p1[1]);
    } else { // parent
        char buffer[5] = {};
        close(p0[0]);
        close(p1[1]);

        write(p0[1], "ping", 4);

        read(p1[0], buffer, 4);
        printf("%d: received %s\n", getpid(), buffer);

        close(p0[1]);
        close(p1[0]);

        wait(0);
    }
    exit(0);
}