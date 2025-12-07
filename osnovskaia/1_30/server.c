#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

#define SOCKET_PATH "/tmp/uppercase_socket"
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    // создать unix domain stream soket
    // AF_UNIX - локальные межпроцессорные сокеты, это семейство адресов,
    // ммежду ними поток sock_stream
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // инициализировать адрес сервера
    memset(&server_addr, 0, sizeof(server_addr));                                 // эта штука обновляет структуру на всякий случай чтобы не было мусора в памяти
    server_addr.sun_family = AF_UNIX;                                             // указывает что это unix domain socket
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1); // копируем путь к сокету в соотв значение

    unlink(SOCKET_PATH); // если был старый файл сокета его удалить

    // привязывает сокет к адресу
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // переход в режимпрослушивания
    if (listen(server_fd, 5) == -1) // 5 - макс размер очереди
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен и слушает на %s\n", SOCKET_PATH);

    // ждем клиента
    client_len = sizeof(client_addr);
    // при подключении создается новый сокет для общения с клиентом
    // а сервер продолжает слушать если еще будут запросы
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1)
    {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Клиент подключен\n");

    // чтение данных от клиента
    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        // преобразование в upper
        for (int i = 0; i < bytes_read; i++)
        {
            buffer[i] = toupper((unsigned char)buffer[i]);
        }
        // сразу выывести
        printf("%s", buffer);
        fflush(stdout);
    }

    if (bytes_read == -1)
    {
        perror("read");
    }

    printf("\nКлиент отключился\n");

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}