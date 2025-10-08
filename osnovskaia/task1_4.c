#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

struct Node
{
    char *str;
    struct Node *next;
    struct Node *prev;
};

int main()
{
    struct Node *head = NULL;
    struct Node **current = &head;
    char buffer[BUFFER_SIZE];

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

        struct Node *new_node = malloc(sizeof(struct Node));

        new_node->str = malloc(len + 1);

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