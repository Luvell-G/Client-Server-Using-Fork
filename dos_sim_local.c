// dos_sim_local.c
// Safe stress-test / DoS simulation client for localhost ONLY.
// Usage: ./dos_sim_local <port> <connections> <hold_seconds>

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int to_int(const char *s, const char *name) {
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (!s || *s == '\0' || (end && *end != '\0') || v <= 0 || v > 1000000) {
        fprintf(stderr, "Invalid %s: %s\n", name, s ? s : "(null)");
        exit(1);
    }
    return (int)v;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <port> <connections> <hold_seconds>\n", argv[0]);
        fprintf(stderr, "Example: %s 8080 200 30\n", argv[0]);
        return 1;
    }

    int port = to_int(argv[1], "port");
    int nconns = to_int(argv[2], "connections");
    int hold = to_int(argv[3], "hold_seconds");

    int *fds = calloc((size_t)nconns, sizeof(int));
    if (!fds) {
        perror("calloc");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    // localhost only:
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        fprintf(stderr, "inet_pton failed\n");
        return 1;
    }

    int opened = 0;
    for (int i = 0; i < nconns; i++) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) {
            perror("socket");
            break;
        }

        if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            // server may start refusing / backlog full / resource limits
            fprintf(stderr, "connect failed at i=%d: %s\n", i, strerror(errno));
            close(s);
            break;
        }

        fds[i] = s;
        opened++;

        // tiny progress print every 25 conns
        if (opened % 25 == 0) {
            printf("Opened %d connections...\n", opened);
            fflush(stdout);
        }
    }

    printf("\nOpened %d/%d connections. Holding sockets open for %d seconds...\n",
           opened, nconns, hold);
    fflush(stdout);

    sleep((unsigned)hold);

    printf("Closing sockets...\n");
    for (int i = 0; i < opened; i++) {
        close(fds[i]);
    }
    free(fds);

    printf("Done.\n");
    return 0;
}
