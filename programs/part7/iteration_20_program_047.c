/* test.c - Simple test program for generating coverage data */
#include <stdio.h>

int function1(int x) {
    if (x > 0) {
        return x * 2;
    } else if (x < 0) {
        return x * -1;
    } else {
        return 0;
    }
}

int function2(int a, int b) {
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
    
    printf("function1(%d) = %d\n", x, function1(x));
    printf("function1(%d) = %d\n", y, function1(y));
    printf("function1(%d) = %d\n", z, function1(z));
    
    printf("function2(%d, %d) = %d\n", 3, 4, function2(3, 4));
    
    return 0;
}
