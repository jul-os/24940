#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
/*
Напишите программу, которая запускает команду, заданную в качестве первого аргумента,
в виде порожденного процесса. Все остальные аргументы программы передаются этой команде.
Затем программа должна дождаться завершения порожденного процесса и распечатать его код
завершения.
*/

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Слишком мало аргументов");
        return 1;
    }
    pid_t pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Ошибка при вызове fork()\n");
        return 1;
    }
    else if (pid == 0) // тут дочерний процесс
    {
        execvp(argv[1], &argv[1]);
        fprintf(stderr, "Ошибка при вызове команды в дочернем процессе\n");
        return 1;
    }
    else // тут родительский процесс
    {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            printf("Дочерний процесс (PID: %d) завершился с кодом: %d\n",
                   pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Процесс погиб от руки сигнала %d\n", WTERMSIG(status));
        }
        else
        {
            printf("\n\nДочерний процесс завершился ненормально\n");
        }
    }
    return 0;
}