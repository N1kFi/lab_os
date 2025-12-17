#include <math.h>
#include "calc.h"

// Первая реализация производной: (f(A + deltaX) - f(A)) / deltaX
float Derivative1(float A, float deltaX) {
    return (cosf(A + deltaX) - cosf(A)) / deltaX;
}

// Первая реализация числа Pi (ряд Лейбница)
float Pi1(int K) {
    if (K <= 0) return 0.0f;

    float pi = 0.0f;
    for (int i = 0; i < K; i++) {
        float term = 1.0f / (2 * i + 1);
        if (i % 2 == 0) {
            pi += term;
        } else {
            pi -= term;
        }
    }
    return 4.0f * pi;
}