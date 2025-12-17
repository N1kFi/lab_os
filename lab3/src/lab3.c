#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define FILENAME "data.bin"
#define MAX_LINES 100
#define MAX_NUMBERS_PER_LINE 100

typedef struct {
    int numbers[MAX_LINES][MAX_NUMBERS_PER_LINE];
    int counts[MAX_LINES];
    int sums[MAX_LINES];
    int line_count;
} shared_data;

int main() {
    size_t file_size = sizeof(shared_data);

    int fd = open(FILENAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, file_size) == -1) {
        perror("ftruncate");
        close(fd);
        exit(EXIT_FAILURE);
    }

    shared_data *data = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    data->line_count = 0;
    for (int i = 0; i < MAX_LINES; i++) {
        data->counts[i] = 0;
        data->sums[i] = 0;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        munmap(data, file_size);
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        while (data->line_count == 0) {
            usleep(100);
        }

        for (int line = 0; line < data->line_count; line++) {
            data->sums[line] = 0;
            for (int i = 0; i < data->counts[line]; i++) {
                data->sums[line] += data->numbers[line][i];
            }
        }

        FILE *result_file = fopen("result.txt", "w");
        if (result_file == NULL) {
            perror("fopen");
            munmap(data, file_size);
            close(fd);
            exit(EXIT_FAILURE);
        }

        for (int line = 0; line < data->line_count; line++) {
            fprintf(result_file, "%d\n", data->sums[line]);
        }
        fclose(result_file);

        munmap(data, file_size);
        close(fd);
        exit(EXIT_SUCCESS);

    } else {
        printf("Введите строки с числами:\n");
        printf("Для завершения введите 'end' или нажмите Ctrl+D:\n");

        char input[256];
        int line = 0;

        while (fgets(input, sizeof(input), stdin) != NULL && line < MAX_LINES) {
            input[strcspn(input, "\n")] = 0;

            if (strlen(input) == 0) {
                continue;
            }

            if (strcmp(input, "end") == 0 || strcmp(input, "END") == 0) {
                printf("Завершение ввода по команде 'end'\n");
                break;
            }

            char *token = strtok(input, " \t");
            int count = 0;

            while (token != NULL && count < MAX_NUMBERS_PER_LINE) {
                if (sscanf(token, "%d", &data->numbers[line][count]) == 1) {
                    count++;
                }
                token = strtok(NULL, " \t");
            }

            if (count > 0) {
                data->counts[line] = count;
                line++;
            } else {
                printf("Пропущена строка без чисел\n");
            }
        }

        data->line_count = line;

        if (data->line_count == 0) {
            printf("Ошибка: не введено ни одной строки.\n");
            munmap(data, file_size);
            close(fd);
            unlink(FILENAME);
            return 1;
        }

        wait(NULL);

        printf("\nРезультаты:\n");
        for (int line = 0; line < data->line_count; line++) {
            printf("Строка %d сумма: %d\n", line + 1, data->sums[line]);
        }

        printf("\nСуммы записаны в result.txt\n");

        munmap(data, file_size);
        close(fd);
        unlink(FILENAME);
    }

    return 0;
}