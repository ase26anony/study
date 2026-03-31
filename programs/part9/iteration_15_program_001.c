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
    
    /* Switch statement */
    switch (y) {
        case 0:
            result += 100;
            break;
        case 1:
            result += 200;
            break;
        case 2:
            result += 300;
            break;
        default:
            result += 400;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int iterations) {
    double total = 0.0;
    volatile int i = 0;
    
    /* While loop */
    while (i < iterations) {
        total += (double)i / 2.0;
        
        /* Multiple conditionals */
        if (i < 5) {
            total += 1.5;
        } else if (i < 10) {
            total += 2.5;
        } else {
            total += 3.5;
        }
        
        i++;
    }
    
    /* Do-while loop */
    do {
        total -= 0.5;
        i--;
    } while (i > 0);
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int value) {
    int local = value;
    
    /* Multiple nested ifs */
    if (depth > 0) {
        local += 10;
        if (depth > 1) {
            local += 20;
            if (depth > 2) {
                local += 30;
            }
        }
    }
    
    /* For loop with break */
    for (volatile int j = 0; j < 10; j++) {
        local += j;
        if (j == depth) {
            break;
        }
    }
    
    return local;
}

/* Function 4: Simple helper */
int func4(volatile int a, volatile int b) {
    return a * b + (a > b ? a : b);
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    /* Parse command line argument for different execution paths */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int result1, result2;
    double result3;
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 0:
            /* Path 0: Call all functions lightly */
            result1 = func1(3, 1);
            result3 = func2(4);
            result2 = func3(2, 10);
            printf("Mode 0: %d, %d, %.2f\n", result1, result2, result3);
            break;
            
        case 1:
            /* Path 1: Different arguments */
            result1 = func1(5, 2);
            result3 = func2(8);
            result2 = func3(3, 20);
            printf("Mode 1: %d, %d, %.2f\n", result1, result2, result3);
            break;
            
        case 2:
            /* Path 2: Skip some functions */
            result1 = func1(2, 0);
            result2 = func4(5, 3);
            printf("Mode 2: %d, %d\n", result1, result2);
            break;
            
        case 3:
            /* Path 3: Deep recursion-like pattern */
            result1 = func1(8, 3);
            for (volatile int i = 0; i < 3; i++) {
                result2 = func3(i, i * 10);
            }
            printf("Mode 3: %d, %d\n", result1, result2);
            break;
            
        default:
            /* Default path */
            result1 = func1(1, 0);
            result3 = func2(2);
            printf("Default: %d, %.2f\n", result1, result3);
    }
    
    /* Always execute func4 */
    result2 = func4(mode, result1 % 10);
    
    return result1 + result2 + (int)result3;
}
