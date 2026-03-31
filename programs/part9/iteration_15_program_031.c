/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    for (volatile int i = 0; i < x; ++i) {
        result += i;
        
        /* Nested conditional */
        if (i % 2 == 0) {
            result *= 2;
        } else {
            result -= 1;
        }
    }
    
    /* Conditional based on y */
    if (y > 10) {
        for (volatile int j = 0; j < 3; ++j) {
            result += y * j;
        }
    } else if (y < 0) {
        result = -result;
    } else {
        result += 100;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile double base, volatile int power) {
    double result = 1.0;
    
    if (power == 0) {
        return 1.0;
    }
    
    /* Loop with break condition */
    for (volatile int i = 0; i < abs(power); ++i) {
        result *= base;
        
        /* Early exit condition */
        if (result > 1000000.0) {
            result = 1000000.0;
            break;
        }
        
        /* Nested switch */
        switch (i % 3) {
            case 0:
                result += 0.1;
                break;
            case 1:
                result -= 0.1;
                break;
            case 2:
                result *= 1.01;
                break;
        }
    }
    
    if (power < 0) {
        result = 1.0 / result;
    }
    
    return result;
}

/* Function 3: Recursive-like pattern with conditionals */
int func3(volatile int n, volatile int depth) {
    static int call_count = 0;
    call_count++;
    
    if (depth <= 0 || n <= 1) {
        return n;
    }
    
    int a = func3(n - 1, depth - 1);
    int b = func3(n - 2, depth - 1);
    
    /* Complex conditional chain */
    if (a > b) {
        return a + (n % 2 == 0 ? 5 : 3);
    } else if (a < b) {
        return b - (n % 3 == 0 ? 2 : 4);
    } else {
        return a * 2;
    }
}

/* Function 4: String processing with loops */
void func4(volatile int mode) {
    char buffer[100];
    volatile int len = 20;
    
    /* Initialize buffer */
    for (volatile int i = 0; i < len; ++i) {
        buffer[i] = 'A' + (i % 26);
    }
    buffer[len] = '\0';
    
    /* Different processing based on mode */
    switch (mode) {
        case 1:
            /* Reverse string */
            for (volatile int i = 0; i < len / 2; ++i) {
                char temp = buffer[i];
                buffer[i] = buffer[len - i - 1];
                buffer[len - i - 1] = temp;
            }
            break;
        case 2:
            /* Shift characters */
            for (volatile int i = 0; i < len; ++i) {
                buffer[i] = ((buffer[i] - 'A' + 3) % 26) + 'A';
            }
            break;
        default:
            /* Count vowels */
            volatile int vowels = 0;
            for (volatile int i = 0; i < len; ++i) {
                char c = buffer[i];
                if (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
                    vowels++;
                }
            }
            break;
    }
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Execute different code paths based on mode */
    switch (mode) {
        case 1:
            printf("Mode 1: Testing func1 and func2\n");
            volatile int r1 = func1(5, 15);
            volatile double r2 = func2(2.0, 3);
            printf("Results: %d, %.2f\n", r1, r2);
            break;
            
        case 2:
            printf("Mode 2: Testing func3 and func4\n");
            volatile int r3 = func3(6, 3);
            func4(1);
            printf("Result: %d\n", r3);
            break;
            
        case 3:
            printf("Mode 3: Testing all functions\n");
            volatile int r4 = func1(3, -5);
            volatile double r5 = func2(1.5, -2);
            volatile int r6 = func3(4, 2);
            func4(2);
            printf("Results: %d, %.2f, %d\n", r4, r5, r6);
            break;
            
        default:
            printf("Default mode: Minimal execution\n");
            volatile int r7 = func1(2, 5);
            printf("Result: %d\n", r7);
            break;
    }
    
    /* Always execute some common code */
    volatile int common = 0;
    for (volatile int i = 0; i < 10; ++i) {
        common += i * mode;
    }
    
    return 0;
}
