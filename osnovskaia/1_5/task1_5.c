#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#define MAX_LINES 1000
#define BUFFER_SIZE 1024

struct line_info
{
    off_t offset;
    size_t length;
};

int check_input(char *input)
{
    int i = 0;
    while (input[i] != '\0')
    {
        if (!isdigit(input[i]))
        {
            printf("ваш ввод содержит что-то, что не цифра. проверьте и попробуйте еще раз\n");
            return 0;
        }
        i++;
    }
    return 1;
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

    // Основной цикл с использованием только системных вызовов для ввода
    char input[100];
    int line_number;

    printf("\nВведите номер строки (0 для выхода): ");

    while (read(STDIN_FILENO, input, sizeof(input)) > 0)
    {

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n')
        {
            input[len - 1] = '\0';
        }

        if (!check_input(input))
        {
             continue;
        }
        else
        {
            line_number = atoi(input);

            if (line_number == 0)
            {
                break;
            }

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
                    printf("Введите номер строки (0 для выхода): ");
                    continue;
                }

                char *line_buffer = malloc(lines[index].length + 1);
                if (line_buffer == NULL)
                {
                    perror("Ошибка выделения памяти");
                    printf("Введите номер строки (0 для выхода): ");
                    continue;
                }

                ssize_t read_bytes = read(fd, line_buffer, lines[index].length);
                if (read_bytes == -1)
                {
                    perror("Ошибка чтения");
                    free(line_buffer);
                    printf("Введите номер строки (0 для выхода): ");
                    continue;
                }

                line_buffer[read_bytes] = '\0';
                printf("Строка %d: %s\n", line_number, line_buffer);
                free(line_buffer);
            }
        }
        printf("Введите номер строки (0 для выхода): ");
    }

    close(fd);
    printf("Программа завершена.\n");
    return 0;
}
