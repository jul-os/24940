#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
/*
Напишите программу, которая создает подпроцесс, взаимодействующий с родителем через
программный канал. Один из процессов выдает в канал текст, состоящий из символов верхнего
и нижнего регистров. Второй процесс переводит все символы в верхний регистр, и выводит
полученный текст на терминал. Подсказка: см. toupper(3).
*/

/*
Неименованный канал создается вызовом pipe, который заносит в массив int [2] два дескриптора
открытых файлов. fd[0] – открыт на чтение, fd[1] – на запись (вспомните STDIN == 0, STDOUT == 1).
Канал уничтожается, когда будут закрыты все файловые дескрипторы ссылающиеся на него.
*/

int main(void)
{
    int fd[2];
    pid_t pid;
    if (pipe(fd) == -1)
    {
        fprintf(stderr, "Ощибка при создании канала\n");
        return 1;
    }
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Ошибка при вызове fork()\n");
        return 1;
    }
    else if (pid == 0) // тут дочерний процесс
    {
        // этот обрабатывать и выводыить через printf, конец на запись в межпроцессовый буфер не нужен
        close(fd[1]);
        char buffer[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(fd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytes_read] = '\0';
            for (int i = 0; i < bytes_read; i++)
            {
                buffer[i] = toupper(buffer[i]);
            }
            printf("%s", buffer);
        }
        close(fd[0]);
    }
    else
    {
        // тут родительский
        // он отправляет текст в медпроцессовый буфер
        close(fd[0]);

        const char *text = "Hello World!\n"
                           "This is a Mixed Case Text.\n"
                           "I'm havin' FUN!\n"
                           "end of transmission\n";

        write(fd[1], text, strlen(text));
        close(fd[1]);
        wait(NULL);
    }

    return 0;
}
