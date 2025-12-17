#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"

int main() {
    printf("Программа со статической линковкой библиотеки\n");
    printf("Используется реализация 1 (ряд Лейбница и первая формула производной)\n");
    printf("Доступные команды:\n");
    printf("  0 - информация о программе\n");
    printf("  1 A deltaX - вычисление производной cos(x) в точке A\n");
    printf("  2 K - вычисление числа Pi с длиной ряда K\n");
    printf("  q - выход из программы\n\n");

    char input[256];
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Удаляем символ новой строки
        input[strcspn(input, "\n")] = 0;

        // Выход из программы
        if (strcmp(input, "q") == 0) {
            break;
        }

        // Информация о программе
        if (strcmp(input, "0") == 0) {
            printf("Программа со статической линковкой библиотеки impl1.so\n");
            printf("Используется реализация 1 (ряд Лейбница и первая формула производной)\n");
            continue;
        }

        // Вычисление производной
        if (input[0] == '1') {
            float A, deltaX;
            if (sscanf(input, "1 %f %f", &A, &deltaX) == 2) {
                float result = Derivative1(A, deltaX);
                printf("Производная cos(x) в точке %.2f: %.6f\n", A, result);
            } else {
                printf("Ошибка: неверные аргументы. Используйте: 1 A deltaX\n");
            }
            continue;
        }

        // Вычисление числа Pi
        if (input[0] == '2') {
            int K;
            if (sscanf(input, "2 %d", &K) == 1) {
                if (K > 0) {
                    float result = Pi1(K);
                    printf("Pi (ряд Лейбница, K=%d): %.6f\n", K, result);
                } else {
                    printf("Ошибка: K должно быть положительным числом\n");
                }
            } else {
                printf("Ошибка: неверные аргументы. Используйте: 2 K\n");
            }
            continue;
        }

        printf("Неизвестная команда. Попробуйте снова.\n");
    }

    return 0;
}