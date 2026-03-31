/* test.c - Simple test program for gcov-dump */
#include <stdio.h>

int func1(int x) {
    if (x > 0) {
        return x * 2;
    } else if (x < 0) {
        return x * -1;
    } else {
        return 0;
    }
}

int func2(int a, int b) {
    int result = 0;
    for (int i = 0; i < a; i++) {
        result += b;
    }
    return result;
}

int main() {
    int x = 5;
    int y = -3;
    int z = 0;
    
    printf("func1(%d) = %d\n", x, func1(x));
    printf("func1(%d) = %d\n", y, func1(y));
    printf("func1(%d) = %d\n", z, func1(z));
    
    printf("func2(%d, %d) = %d\n", 3, 4, func2(3, 4));
    
    return 0;
}
