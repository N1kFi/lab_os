#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <math.h>

// Указатели на функции
static float (*DerivativeFunc)(float A, float deltaX) = NULL;
static float (*PiFunc)(int K) = NULL;

// Глобальные переменные
static void* lib_handle = NULL;
static int current_impl = 1;  // Текущая реализация: 1 или 2

// Функция загрузки библиотеки
int load_library(const char* lib_name) {
    // Закрываем предыдущую библиотеку, если она была открыта
    if (lib_handle != NULL) {
        dlclose(lib_handle);
    }

    // Открываем новую библиотеку
    lib_handle = dlopen(lib_name, RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Ошибка загрузки библиотеки %s: %s\n", lib_name, dlerror());
        return 0;
    }

    // Загружаем функции в зависимости от текущей реализации
    if (current_impl == 1) {
        DerivativeFunc = dlsym(lib_handle, "Derivative1");
        PiFunc = dlsym(lib_handle, "Pi1");
    } else {
        DerivativeFunc = dlsym(lib_handle, "Derivative2");
        PiFunc = dlsym(lib_handle, "Pi2");
    }

    if (!DerivativeFunc || !PiFunc) {
        fprintf(stderr, "Ошибка загрузки функций: %s\n", dlerror());
        dlclose(lib_handle);
        lib_handle = NULL;
        return 0;
    }

    return 1;
}

// Функция переключения между реализациями
void switch_implementation() {
    current_impl = (current_impl == 1) ? 2 : 1;

    const char* lib_name = (current_impl == 1) ? "./libimpl1.so" : "./libimpl2.so";

    if (load_library(lib_name)) {
        printf("Переключено на реализацию %d\n", current_impl);
        if (current_impl == 1) {
            printf("  Производная: (f(A + deltaX) - f(A)) / deltaX\n");
            printf("  Число Pi: ряд Лейбница\n");
        } else {
            printf("  Производная: (f(A + deltaX) - f(A - deltaX)) / (2 * deltaX)\n");
            printf("  Число Pi: формула Валлиса\n");
        }
    }
}

int main() {
    printf("Программа с динамической загрузкой библиотек\n");
    printf("Доступные команды:\n");
    printf("  0 - переключить реализацию\n");
    printf("  1 A deltaX - вычисление производной cos(x) в точке A\n");
    printf("  2 K - вычисление числа Pi с длиной ряда K\n");
    printf("  q - выход из программы\n\n");

    // Загружаем первую библиотеку по умолчанию
    if (!load_library("./libimpl1.so")) {
        fprintf(stderr, "Не удалось загрузить библиотеку по умолчанию\n");
        return 1;
    }

    printf("Загружена реализация 1 (ряд Лейбница и первая формула производной)\n\n");

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

        // Переключение реализации
        if (strcmp(input, "0") == 0) {
            switch_implementation();
            continue;
        }

        // Вычисление производной
        if (input[0] == '1') {
            float A, deltaX;
            if (sscanf(input, "1 %f %f", &A, &deltaX) == 2) {
                if (deltaX == 0) {
                    printf("Ошибка: deltaX не может быть равно 0\n");
                } else {
                    float result = DerivativeFunc(A, deltaX);
                    printf("Производная cos(x) в точке %.2f: %.6f\n", A, result);
                }
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
                    float result = PiFunc(K);
                    const char* method = (current_impl == 1) ? "ряд Лейбница" : "формула Валлиса";
                    printf("Pi (%s, K=%d): %.6f\n", method, K, result);
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

    // Закрываем библиотеку перед выходом
    if (lib_handle != NULL) {
        dlclose(lib_handle);
    }

    return 0;
}