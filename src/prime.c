#include "prime.h"
#include <math.h>

int is_prime(const int x) {
    if (x < 2) return 0;
    if (x == 2 || x == 3) return 1;
    if (x % 2 == 0 || x % 3 == 0) return 0;

    for (int i = 5; i * i <= x; i += 6) {
        if (x % i == 0 || x % (i + 2) == 0) return 0;
    }
    return 1;
}

int next_prime(const int x) {
    int n = x;
    if (n % 2 == 0 && n > 2) n++;
    while (!is_prime(n)) {
        n += 2;
    }
    return n;
}