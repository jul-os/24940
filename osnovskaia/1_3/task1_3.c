#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    uid_t effective_uid = geteuid();
    uid_t real_uid = getuid();
    printf("effective id's: uid=%d\nreal id's: uid=%d\n", (int)effective_uid, (int)real_uid);
    FILE *o;
    if ((o = fopen("file.txt", "r")) == NULL)
    {
        perror("couldn't open the file\n");
        return 1;
    }
    fclose(o);
    setuid(real_uid);
    printf("effective id's: uid=%d\nreal id's: uid=%d\n", (int)effective_uid, (int)real_uid);
    if ((o = fopen("file.txt", "r")) == NULL)
    {
        perror("couldn't open the file\n");
        return 1;
    }
    fclose(o);
}