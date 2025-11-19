#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t signal_count = 0;

void sigint_handler(int sig) {
    (void)sig;
    signal_count++;
    putchar('\a');
    fflush(stdout);
}

void sigquit_handler(int sig) {
    (void)sig;
    printf("Signal sounded %d time(s).\n", signal_count);
    exit(0);
}

int main(void) {
    signal(SIGQUIT, sigint_handler); // Ctrl+C
    signal(SIGINT, sigquit_handler); // Ctrl+4
    
    while (1) {
        pause();
    }
}
