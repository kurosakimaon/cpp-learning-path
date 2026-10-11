/*
 * 数学计算库实现
 */

#include "math_utils.h"

namespace mathutils {

int add(int a, int b) { return a + b; }
int subtract(int a, int b) { return a - b; }
int multiply(int a, int b) { return a * b; }

int divide(int a, int b) {
    if (b == 0) return 0;
    return a / b;
}

long long factorial(int n) {
    if (n <= 1) return 1;
    long long result = 1;
    for (int i = 2; i <= n; i++) result *= i;
    return result;
}

bool isPrime(int n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (int i = 3; (long long)i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

} // namespace mathutils
