#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 256

struct Node
{
    char *str;
    struct Node *next;
    struct Node *prev;
};

int is_valid_string(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        unsigned char c = str[i];
        // Разрешаем буквы, цифры, пробел и основные знаки препинания
        if (!isalnum(c) && c != ' ' && c != '.' && c != ',' && c != '!' &&
            c != '?' && c != '-' && c != '_' && c != '(' && c != ')' &&
            c != ':' && c != ';' && c != '\'' && c != '\"')
        {
            return 0;
        }
    }
    return 1;
}

void how_to_use()
{
    printf("Разрешены буквы, цифры, пробел, основные знаки препинания\nСтроки, в которых встретится запрещенный символ, будут проигнорированы\nЧтобы завершить ввод введите строку, начинающуюся на точку\n");
}

int main()
{

    struct Node *head = NULL;
    struct Node **current = &head;
    char buffer[BUFFER_SIZE];
    how_to_use();
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
            break;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
            len--;
        }

        if (len > 0 && buffer[0] == '.')
        {
            break;
        }

        // пустые строки и строки с запрещенными символами будут пропущены
        if (len == 0 || !is_valid_string(buffer))
        {
            continue;
        }

        struct Node *new_node = malloc(sizeof(struct Node));
        if (new_node == NULL)
        {
            fprintf(stderr, "Memory allocation error\n");
            break;
        }

        new_node->str = malloc(len + 1);
        if (new_node->str == NULL)
        {
            fprintf(stderr, "Memory allocation error\n");
            free(new_node);
            break;
        }

        strcpy(new_node->str, buffer);
        new_node->next = NULL;

        *current = new_node;
        current = &(*current)->next;
    }

    struct Node *ptr = head;
    int i = 1;
    while (ptr != NULL)
    {
        printf("%d: %s\n", i++, ptr->str);
        ptr = ptr->next;
    }

    ptr = head;
    while (ptr != NULL)
    {
        struct Node *temp = ptr;
        ptr = ptr->next;
        free(temp->str);
        free(temp);
    }

    return 0;
}