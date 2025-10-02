#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    int pipe1[2];
    if (pipe(pipe1) == -1) {
        perror("pipe1");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        close(pipe1[1]);

        char *args[] = {"./child", NULL};
        execv(args[0], args);
        perror("execv");
        exit(1);
    }
    else {
        close(pipe1[0]);
        char filename[256];

        printf("Введите имя файла для записи: ");
        fflush(stdout);
        if (scanf("%255s", filename) != 1) {
            fprintf(stderr, "Ошибка ввода имени файла\n");
            exit(1);
        }

        dprintf(pipe1[1], "%s\n", filename);

        printf("Введите строки с числами (для выхода Ctrl+D):\n");
        getchar();

        char buffer[512];
        while (fgets(buffer, sizeof(buffer), stdin)) {
            write(pipe1[1], buffer, strlen(buffer));
        }

        close(pipe1[1]);
        wait(NULL);
        printf("Дочерний процесс завершён.\n");
    }
    return 0;
}