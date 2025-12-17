#include <math.h>
#include "calc.h"

// Вторая реализация производной: (f(A + deltaX) - f(A - deltaX)) / (2 * deltaX)
float Derivative2(float A, float deltaX) {
    return (cosf(A + deltaX) - cosf(A - deltaX)) / (2 * deltaX);
}

// Вторая реализация числа Pi (формула Валлиса)
float Pi2(int K) {
    if (K <= 0) return 0.0f;

    float pi = 1.0f;
    for (int i = 1; i <= K; i++) {
        float numerator = 4.0f * i * i;
        float denominator = 4.0f * i * i - 1.0f;
        pi *= numerator / denominator;
    }
    return 2.0f * pi;
}