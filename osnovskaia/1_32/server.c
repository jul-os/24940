#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <aio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define SOCKET_PATH "my_socket"
#define MAX_CLIENTS 20
#define READ_SIZE 1

struct client
{
    int fd;
    struct aiocb cb;
    char byte;
    int active;
    int id; // номер клиента
};

static struct client clients[MAX_CLIENTS];

void print_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", localtime(&ts.tv_sec));
    printf("[%s.%03ld] ", time_buf, ts.tv_nsec / 1000000);
}

static void cleanup(int sig)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].fd != -1)
        {
            aio_cancel(clients[i].fd, NULL);
            close(clients[i].fd);
        }
    unlink(SOCKET_PATH);
    exit(0);
}

int main(void)
{
    int listen_fd, new_fd;
    struct sockaddr_un addr;
    int slot;

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(SOCKET_PATH);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    if (listen(listen_fd, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    fcntl(listen_fd, F_SETFL, O_NONBLOCK);

    printf("Task 32 server — POSIX AIO + CHAOTIC BYTE MIXING (with timestamps) running...\n");

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].fd = -1;
        clients[i].id = i + 1;
    }

    while (1)
    {
        while ((new_fd = accept(listen_fd, NULL, NULL)) != -1)
        {
            for (slot = 0; slot < MAX_CLIENTS; slot++)
                if (clients[slot].fd == -1)
                    break;
            if (slot == MAX_CLIENTS)
            {
                close(new_fd);
                continue;
            }

            clients[slot].fd = new_fd;
            clients[slot].active = 1;

            memset(&clients[slot].cb, 0, sizeof(struct aiocb));
            clients[slot].cb.aio_fildes = new_fd;
            clients[slot].cb.aio_buf = &clients[slot].byte;
            clients[slot].cb.aio_nbytes = READ_SIZE;

            if (aio_read(&clients[slot].cb) == -1)
            {
                perror("aio_read");
                close(new_fd);
                clients[slot].fd = -1;
                clients[slot].active = 0;
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (!clients[i].active)
                continue;

            int err = aio_error(&clients[i].cb);
            if (err == EINPROGRESS)
                continue;

            ssize_t ret = aio_return(&clients[i].cb);

            if (ret == 1)
            {
                char c = toupper((unsigned char)clients[i].byte);
                printf("%c", c);

                if (aio_read(&clients[i].cb) == -1 && errno != EAGAIN && errno != EINTR)
                {
                    if (errno != EAGAIN && errno != EINTR)
                    {
                        close(clients[i].fd);
                        clients[i].fd = -1;
                        clients[i].active = 0;
                    }
                }
            }
            else
            {
                close(clients[i].fd);
                clients[i].fd = -1;
                clients[i].active = 0;
            }
        }
        usleep(1000);
    }
    return 0;
}