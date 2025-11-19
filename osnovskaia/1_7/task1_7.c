#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <ctype.h>

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

int check_line_num(int line_number, int line_count)
{
    if (line_number == 0)
    {
        printf("Выход по запросу пользователя.\n");
        return 0;
    }
    if (line_number < 1 || line_number > line_count)
    {
        printf("Ошибка: строка %d не существует. Доступные строки: 1-%d\n",
               line_number, line_count);

        return 1;
    }
    return 2;
}

void read_from_file(int line_number, struct line_info lines[], char *file_data, int line_count, int file_size)
{
    int check_result = check_line_num(line_number, line_count);
    if (check_result == 0)
    {
        // Освобождаем отображенную память
        if (munmap(file_data, file_size) == -1)
        {
            perror("Ошибка освобождения отображенной памяти");
        }
        exit(0);
    }
    if (check_result == 1)
        return;

    int index = line_number - 1;

    // Получаем указатель на начало строки в отображенной памяти
    char *line_start_ptr = file_data + lines[index].offset;
    // Создаем буфер для строки и копируем данные
    char *line_buffer = malloc(lines[index].length + 1);
    if (line_buffer != NULL)
    {
        // Копируем строку из отображенной памяти
        memcpy(line_buffer, line_start_ptr, lines[index].length);
        line_buffer[lines[index].length] = '\0';
        printf("Строка %d: %s", line_number, line_buffer);
        free(line_buffer);
    }
    else
    {
        perror("Ошибка выделения памяти");
    }
    return;
}

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

// Функция для вывода всего содержимого файла
void print_entire_file(char *file_data, size_t file_size)
{
    printf("\nВремя вышло! Вывод всего содержимого файла:\n");
    printf("============================================\n");

    write(STDOUT_FILENO, file_data, file_size);

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
    // Получаем размер файла
    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        perror("Ошибка получения информации о файле");
        close(fd);
        return 1;
    }
    size_t file_size = sb.st_size;

    // Если файл пустой
    if (file_size == 0)
    {
        printf("Файл пустой.\n");
        close(fd);
        return 0;
    }
    // Отображаем файл в память
    char *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED)
    {
        perror("Ошибка отображения файла в память");
        close(fd);
        return 1;
    }

    // Закрываем файловый дескриптор - он больше не нужен
    close(fd);
    struct line_info lines[MAX_LINES];
    int line_count = 0;
    char buffer[BUFFER_SIZE];
    off_t line_start = 0;
    size_t line_length = 0;
    ssize_t bytes_read;

    printf("Построение таблицы строк...\n");

    for (ssize_t i = 0; i < file_size; i++)
    {
        if (file_data[i] == '\n')
        {
            if (line_count < MAX_LINES)
            {
                lines[line_count].offset = line_start;
                lines[line_count].length = line_length;
                line_count++;
            }
            line_start = i + 1;
            line_length = 0;
        }
        else
        {
            line_length++;
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
        printf("%d\t%zu\t\t%zu\n", (i + 1), lines[i].offset, lines[i].length);
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
        print_entire_file(file_data, file_size);
        close(fd);
        return 0;
    }

    input[bytes_read_input] = '\0';
    if (bytes_read_input > 0 && input[bytes_read_input - 1] == '\n')
    {
        input[bytes_read_input - 1] = '\0';
        bytes_read_input--;
    }
    if (check_input(input))
    {
        line_number = atoi(input);
        read_from_file(line_number, lines, file_data, line_count, file_size);
    }

    while (1)
    {

        printf("\nВведите номер строки (0 для выхода): ");

        bytes_read_input = read(STDIN_FILENO, input, sizeof(input) - 1);

        if (bytes_read_input <= 0)
            break;

        input[bytes_read_input] = '\0';
        if (bytes_read_input > 0 && input[bytes_read_input - 1] == '\n')
        {
            input[bytes_read_input - 1] = '\0';
            bytes_read_input--;
        }
        if (!check_input(input))
        {
            continue;
        }
        else
        {
            line_number = atoi(input);
            read_from_file(line_number, lines, file_data, line_count, file_size);
        }
    }
    // Освобождаем отображенную память
    if (munmap(file_data, file_size) == -1)
    {
        perror("Ошибка освобождения отображенной памяти");
    }
    close(fd);
    return 0;
}