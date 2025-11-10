#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int beep_count = 0;

void signal_handler(int sig)
{
    if (sig == SIGINT)
    {
        beep_count++;
        write(STDOUT_FILENO, "\a", 1); // звуковой сигнал
    }
    else if (sig == SIGQUIT)
    {
        printf("\nПрограмма завершена. Всего звуковых сигналов: %d\n", beep_count);
        exit(0);
    }
}

int main()
{
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    printf("Программа запущена. Используйте:\n");
    printf("  Ctrl-C - звуковой сигнал\n");
    printf("  Ctrl-\\ - завершение программы\n");
    printf("Ожидание сигналов...\n");
    while (1)
    {
        pause(); // ожидает любой сигнал
    }

    return 0;
}