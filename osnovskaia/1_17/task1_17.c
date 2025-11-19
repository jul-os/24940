#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 40
#define ERASE 0x7F
#define KILL 0x15
#define CTRL_W 0x17
#define CTRL_D 0x04
#define BELL 0x07

char buf[256];
int len = 0;
int col = 0;

// Функция для поиска начала последнего слова
int find_last_word_start()
{
    if (len == 0)
        return 0;

    int pos = len - 1;

    // Пропускаем пробелы в конце
    while (pos >= 0 && isspace(buf[pos]))
        pos--;

    // Если только пробелы или пустая строка
    if (pos < 0)
        return len;

    // Ищем начало слова
    while (pos >= 0 && !isspace(buf[pos]))
        pos--;

    return pos + 1;
}

void del_word()
{
    if (len == 0)
    {
        putchar(BELL);
        fflush(stdout);
        return;
    }

    int word_start = find_last_word_start();
    len = word_start;
}

void init_term()
{
    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &t);
}

void reterm()
{
    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr(0, TCSANOW, &t);
}

void redraw()
{
    int i;
    for (i = 0; i < len + 2; i++)
        putchar('\b');
    for (i = 0; i < len + 2; i++)
        putchar(' ');
    for (i = 0; i < len + 2; i++)
        putchar('\b');

    col = 0;
    for (i = 0; i < len; i++)
    {
        putchar(buf[i]);
        col++;
        if (col >= MAX_LEN && i < len - 1)
        {
            putchar('\n');
            col = 0;
        }
    }
    fflush(stdout);
}

int main()
{
    init_term();

    char c;
    while (1)
    {
        c = getchar();

        if (c == CTRL_D && len == 0)
            break;

        if (c == ERASE)
        {
            if (len > 0)
                len--;
            else
                putchar(BELL);
        }
        else if (c == KILL)
        {
            len = 0;
        }
        else if (c == CTRL_W)
        {
            del_word();
        }
        else if (c >= 32 && c <= 126)
        {
            if (len < 255)
                buf[len++] = c;
            else
                putchar(BELL);
        }
        else
        {
            putchar(BELL);
            continue;
        }

        redraw();
    }

    reterm();
    printf("\n");
    return 0;
}