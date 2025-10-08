#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#define MAX_LINES 1000
#define BUFFER_SIZE 1024
#define TIMEOUT 5

struct line_info
{
    off_t offset;
    size_t length;
};

volatile sig_atomic_t timeout_occurred = 0;

// Обработчик сигнала ALARM
void alarm_handler(int sig)
{
    timeout_occurred = 1;
}

// Функция для вывода всего содержимого файла
void print_entire_file(int fd)
{
    printf("\nВремя вышло! Вывод всего содержимого файла:\n");
    printf("============================================\n");

    // Перемещаемся в начало файла
    if (lseek(fd, 0, SEEK_SET) == -1)
    {
        perror("Ошибка позиционирования в начале файла");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Читаем и выводим весь файл
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0)
    {
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    printf("\n============================================\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        write(STDERR_FILENO, "Использование: program <filename>\n", 35);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("Ошибка открытия файла");
        return 1;
    }

    struct line_info lines[MAX_LINES];
    int line_count = 0;
    char buffer[BUFFER_SIZE];
    off_t line_start = 0;
    size_t line_length = 0;
    ssize_t bytes_read;

    printf("Построение таблицы строк...\n");

    // Буферизованное чтение для построения таблицы
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0)
    {
        for (ssize_t i = 0; i < bytes_read; i++)
        {
            if (buffer[i] == '\n')
            {
                if (line_count < MAX_LINES)
                {
                    lines[line_count].offset = line_start;
                    lines[line_count].length = line_length;
                    line_count++;
                }
                line_start = lseek(fd, 0L, SEEK_CUR) - bytes_read + i + 1;
                line_length = 0;
            }
            else
            {
                line_length++;
            }
        }
    }

    // Обработка последней строки
    if (line_length > 0 && line_count < MAX_LINES)
    {
        lines[line_count].offset = line_start;
        lines[line_count].length = line_length;
        line_count++;
    }

    // Вывод таблицы для отладки
    printf("\nТаблица строк:\n");
    printf("Строка\tСмещение\tДлина\n");
    for (int i = 0; i < line_count; i++)
    {
        printf("%d\t%ld\t\t%zu\n", i + 1, (long)lines[i].offset, lines[i].length);
    }

    // Настройка обработчика сигнала
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        perror("Ошибка настройки обработчика сигнала");
        close(fd);
        return 1;
    }

    // Основной цикл с таймаутом
    char input[100];
    int line_number;

    printf("\nУ вас есть %d секунд для ввода номера строки...\n", TIMEOUT);
    printf("Введите номер строки (0 для выхода): ");

    // Устанавливаем буферизацию для немедленного вывода
    setbuf(stdout, NULL);

    // Устанавливаем таймер на 5 секунд
    alarm(TIMEOUT);

    ssize_t bytes_read_input = read(STDIN_FILENO, input, sizeof(input) - 1);

    // Отменяем таймер, так как ввод произошел
    alarm(0);

    if (timeout_occurred || bytes_read_input <= 0)
    {
        // Таймаут или ошибка ввода - выводим весь файл
        print_entire_file(fd);
        close(fd);
        return 0;
    }

    // Обрабатываем введенные данные
    input[bytes_read_input] = '\0';
    line_number = atoi(input);

    if (line_number == 0)
    {
        printf("Выход по запросу пользователя.\n");
        close(fd);
        return 0;
    }

    while (1)
    {
        if (line_number < 1 || line_number > line_count)
        {
            printf("Ошибка: строка %d не существует. Доступные строки: 1-%d\n",
                   line_number, line_count);
        }
        else
        {
            int index = line_number - 1;

            // Позиционируемся и читаем строку
            if (lseek(fd, lines[index].offset, SEEK_SET) == -1)
            {
                perror("Ошибка позиционирования");
            }
            else
            {
                char *line_buffer = malloc(lines[index].length + 1);
                if (line_buffer != NULL)
                {
                    ssize_t read_bytes = read(fd, line_buffer, lines[index].length);
                    if (read_bytes != -1)
                    {
                        line_buffer[read_bytes] = '\0';
                        printf("Строка %d: %s\n", line_number, line_buffer);
                    }
                    free(line_buffer);
                }
            }
        }

        printf("\nВведите номер строки (0 для выхода): ");

        // Сбрасываем флаг таймаута и устанавливаем новый таймер
        timeout_occurred = 0;
        alarm(TIMEOUT);

        bytes_read_input = read(STDIN_FILENO, input, sizeof(input) - 1);
        alarm(0); // Отменяем таймер

        if (timeout_occurred)
        {
            print_entire_file(fd);
            break;
        }

        if (bytes_read_input <= 0)
            break;

        input[bytes_read_input] = '\0';
        line_number = atoi(input);

        if (line_number == 0)
        {
            printf("Программа завершена.\n");
            break;
        }
    }
    close(fd);
    return 0;
}