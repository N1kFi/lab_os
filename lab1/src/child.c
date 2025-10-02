#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char filename[256];

    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        fprintf(stderr, "Не получено имя файла\n");
        exit(1);
    }
    filename[strcspn(filename, "\n")] = 0;

    FILE *f = fopen(filename, "a");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    char line[512];
    while (fgets(line, sizeof(line), stdin)) {
        int sum = 0, x;
        char *ptr = line;
        while (sscanf(ptr, "%d", &x) == 1) {
            sum += x;
            while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n') ptr++;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
        }
        fprintf(f, "%d\n", sum);
        fflush(f);
    }

    fclose(f);
    return 0;
}
