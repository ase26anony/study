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
    
    /* Another loop */
    volatile int j = y;
    while (j > 0) {
        result += j;
        if (result > 100) {
            result = 100;  /* Cap the result */
        }
        j--;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int n, volatile double factor) {
    double total = 0.0;
    
    if (n <= 0) {
        return 0.0;
    }
    
    /* Switch-like logic with if-else chain */
    if (n < 10) {
        for (volatile int i = 0; i < n; ++i) {
            total += i * factor;
        }
    } else if (n < 20) {
        volatile int k = n;
        do {
            total += k * factor * 0.5;
            k--;
        } while (k > 0);
    } else {
        total = n * factor * 2.0;
    }
    
    /* Final adjustment */
    if (total > 50.0) {
        total -= 25.0;
    }
    
    return total;
}

/* Function 3: More branching */
char* func3(volatile int mode, volatile int count) {
    static char buffer[100];
    int idx = 0;
    
    switch (mode) {
        case 1:
            for (volatile int i = 0; i < count; ++i) {
                buffer[idx++] = 'A' + (i % 26);
            }
            break;
        case 2:
            for (volatile int i = count; i > 0; --i) {
                buffer[idx++] = 'Z' - (i % 26);
            }
            break;
        default:
            for (volatile int i = 0; i < 10; ++i) {
                buffer[idx++] = '0' + (i % 10);
            }
            break;
    }
    
    buffer[idx] = '\0';
    return buffer;
}

/* Function 4: Recursive-like pattern */
int func4(volatile int depth, volatile int value) {
    int result = value;
    
    if (depth <= 0) {
        return result;
    }
    
    /* Multiple condition checks */
    if (value > 0) {
        for (volatile int i = 0; i < depth; ++i) {
            if (i % 3 == 0) {
                result += 5;
            } else if (i % 3 == 1) {
                result -= 2;
            } else {
                result *= 1;
            }
        }
    } else {
        result = -value;
        volatile int temp = depth;
        while (temp > 0) {
            result += temp;
            temp /= 2;
        }
    }
    
    return result;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 1;
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    /* Execute different code paths based on argument */
    if (arg == 1) {
        /* Path 1: Call all functions */
        int r1 = func1(5, 3);
        double r2 = func2(8, 1.5);
        char* r3 = func3(1, 5);
        int r4 = func4(3, 10);
        
        printf("Path 1: %d, %.2f, %s, %d\n", r1, r2, r3, r4);
    } 
    else if (arg == 2) {
        /* Path 2: Different arguments */
        int r1 = func1(3, 7);
        double r2 = func2(15, 0.8);
        char* r3 = func3(2, 4);
        int r4 = func4(2, -5);
        
        printf("Path 2: %d, %.2f, %s, %d\n", r1, r2, r3, r4);
    }
    else if (arg == 3) {
        /* Path 3: Skip some functions */
        int r1 = func1(10, 2);
        double r2 = func2(25, 2.0);
        
        printf("Path 3: %d, %.2f\n", r1, r2);
    }
    else {
        /* Default path: Minimal execution */
        int r1 = func1(1, 1);
        printf("Default: %d\n", r1);
    }
    
    /* Always execute this common code */
    volatile int common = 5;
    for (volatile int i = 0; i < common; ++i) {
        /* Empty loop for coverage arcs */
    }
    
    return 0;
}
