#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <bits/getopt_core.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>
// todo print usage

void print_usage()
{
}
void set_environment_variable(const char *name, const char *value)
{
    printf("\n");
    if (setenv(name, value, 1) == -1)
    {
        printf("Couldn't set env var\n");
        return;
    }
    printf("Environment variable %s set to %s\n", name, value);
}
int parse_name_value(const char *input, char **name, char **value)
{
    char *equals = strchr(input, '=');
    if (equals == NULL)
    {
        printf("Invalid format for -V. Use: name=value\n");
        return -1;
    }
    size_t name_len = equals - input;
    *name = malloc(name_len + 1);
    *value = strdup(equals + 1);
    if (*name == NULL || *value == NULL)
    {
        printf("malloc/strdup failure\n");
        free(*name);
        *name = NULL;
        free(*value);
        *value = NULL;
        return -1;
    }
    strncpy(*name, input, name_len);
    (*name)[name_len] = '\0';
    return 0;
}
// todo в функции возможно
extern char **environ;
int main(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1)
    {
        switch (c)
        {
        case 'i':
        {
            // ask как можно отследить изменения реального и эффективного
            uid_t effective_uid = geteuid();
            gid_t effective_gid = getegid();
            uid_t real_uid = getuid();
            gid_t real_gid = getgid();
            printf("effective id's: uid=%d, gid=%d \nreal id's: uid=%d, gid=%d", (int)effective_uid, (int)effective_gid, (int)real_uid, (int)real_gid);
        }
        break;
        case 's':
        {
            if (setpgid(0, 0) == -1)
            {
                printf("error setting group leader\n");
                return 1;
            }
            printf("Process became new group leader\n");
            // 0 - текущий процесс
            // в поле для группы у него тоже 0 потому что он не смодет стать лидером своей текущей
            // группы если он уже им не является а если является то он останется как был и все
        }
        break;
        case 'p':
        {
            pid_t pid = getpid();
            printf("Process id: %d, parent process id:%d, group process id:%d", (int)pid, (int)getppid(), (int)getpgid(pid));
        }
        break;
        case 'u':
        {
            struct rlimit rlp;
            if (getrlimit(RLIMIT_FSIZE, &rlp) == -1)
            {
                perror("getrlimit");
                return 1;
            }
            printf("ulimit: %ld\n", (long)rlp.rlim_cur);
        }
        break;
        case 'U':
        {
            // Для опции с аргументом используем optarg
            if (optarg != NULL)
            {
                long new_limit = atol(optarg);
                struct rlimit rlp;
                if (getrlimit(RLIMIT_FSIZE, &rlp) == -1)
                {
                    perror("getrlimit");
                    return 1;
                }
                rlp.rlim_cur = new_limit;
                if (setrlimit(RLIMIT_FSIZE, &rlp) == -1)
                {
                    perror("setrlimit");
                    return 1;
                }
                printf("New ulimit set to: %ld\n", new_limit);
            }
            else
            {
                fprintf(stderr, "Option -U requires an argument\n");
                return 1;
            }
        }
        break;
        /*
        Core-файл (дамп памяти) — это "моментальный снимок" оперативной памяти программы
        в тот самый момент, когда она аварийно завершилась (упала).
        */
        case 'c':
        {
            struct rlimit rlim;
            if (getrlimit(RLIMIT_CORE, &rlim) == -1)
            {
                perror("getrlimit failed");
                return 1;
            }
            printf("Current core file size limit: ");
            if (rlim.rlim_cur == RLIM_INFINITY)
            {
                printf("unlimited\n");
            }
            else
            {
                printf("%ld bytes\n", (long)rlim.rlim_cur);
            }
        }
        break;
        case 'C':
        {
            if (optarg == NULL)
            {
                fprintf(stderr, "Option -C requires an argument\n");
                return 1;
            }
            // Преобразуем строку в число
            char *endptr;
            long size = strtol(optarg, &endptr, 10);
            // Проверяем корректность преобразования
            if (endptr == optarg || *endptr != '\0')
            {
                fprintf(stderr, "Invalid number for -C: %s\n", optarg);
                return 1;
            }
            // Проверяем на отрицательные значения
            if (size < 0)
            {
                fprintf(stderr, "Core file size cannot be negative: %ld\n", size);
                return 1;
            }
            struct rlimit rlim;
            if (getrlimit(RLIMIT_CORE, &rlim) == -1)
            {
                perror("getrlimit failed");
                return 1;
            }
            rlim.rlim_cur = (rlim_t)size;
            if (setrlimit(RLIMIT_CORE, &rlim) == -1)
            {
                perror("setrlimit failed");
                return 1;
            }
            printf("Core file size limit set to: ");
            if (rlim.rlim_cur == RLIM_INFINITY)
            {
                printf("unlimited\n");
            }
            else
            {
                printf("%ld bytes\n", (long)rlim.rlim_cur);
            }
        }
        break;
        case 'd':
        {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                printf("Current directory: %s\n", cwd);
            }
            else
            {
                perror("getcwd() error");
            }
        }
        break;
        case 'v':
        {
            printf("Environment variables:\n");
            printf("=====================\n");
            for (char **env = environ; *env != NULL; env++)
            {
                printf("%s\n", *env);
            }
            printf("=====================\n");
        }
        break;
        case 'V':
        {
            char *name, *value;
            if (parse_name_value(optarg, &name, &value) == -1)
            {
                break;
            }
            set_environment_variable(name, value);
            free(name);
            free(value);
        }
        break;
        case '?':
            fprintf(stderr, "Invalid option: %c\n", optopt);
            break;
        default:
            fprintf(stderr, "Unexpected error\n");
            break;
        }
    }
    // Если опций нет
    if (optind == 1)
    {
        printf("No options provided. Use -d to print current directory.\n");
    }
    return 0;
}