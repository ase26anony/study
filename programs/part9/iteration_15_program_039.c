/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int complex_func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    for (volatile int i = 0; i < x; ++i) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < y; ++j) {
            result += (i * j) % 5;
        }
    }
    
    /* Conditional chain */
    if (x > 10) {
        result *= 2;
    } else if (x > 5) {
        result += 100;
    } else {
        result -= 50;
    }
    
    return result;
}

/* Function 2: Different control flow pattern */
double complex_func2(volatile double base, volatile int power) {
    double result = 1.0;
    
    if (power < 0) {
        base = 1.0 / base;
        power = -power;
    }
    
    /* While loop */
    volatile int count = 0;
    while (count < power) {
        result *= base;
        count++;
        
        /* Switch inside loop */
        switch (count % 3) {
            case 0:
                result += 0.1;
                break;
            case 1:
                result -= 0.05;
                break;
            case 2:
                result *= 1.01;
                break;
        }
    }
    
    return result;
}

/* Function 3: Recursive-like pattern with goto */
int complex_func3(volatile int n, volatile int depth) {
    int total = 0;
    
    if (depth > 5) {
        return n;
    }
    
    /* Use goto for unusual control flow */
    volatile int i = 0;
start_loop:
    if (i >= n) goto end_loop;
    
    total += i;
    
    if (i % 3 == 0) {
        total += complex_func3(i, depth + 1);
    } else if (i % 3 == 1) {
        total *= 2;
    }
    
    i++;
    goto start_loop;
    
end_loop:
    
    /* Do-while loop */
    volatile int j = 0;
    do {
        total -= j;
        j++;
    } while (j < 3);
    
    return total;
}

/* Function 4: Matrix-like operations */
void complex_func4(volatile int size, int* output) {
    /* Two-dimensional loop pattern */
    for (volatile int row = 0; row < size; ++row) {
        for (volatile int col = 0; col < size; ++col) {
            int idx = row * size + col;
            
            /* Multiple condition checks */
            if (row == col) {
                output[idx] = 1;
            } else if (row > col) {
                output[idx] = 2;
            } else {
                output[idx] = 3;
            }
            
            /* Early continue */
            if ((row + col) % 7 == 0) {
                output[idx] *= -1;
                continue;
            }
            
            /* Break from inner loop under condition */
            if (output[idx] > 100) {
                break;
            }
        }
        
        /* Break from outer loop */
        if (row >= 10) {
            break;
        }
    }
}

/* Main function with different execution paths */
int main(int argc, char* argv[]) {
    volatile int mode = 0;
    
    /* Parse command line for different execution paths */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int result1, result3;
    double result2;
    int matrix[100];
    
    /* Execute different code paths based on mode */
    switch (mode % 4) {
        case 0:
            result1 = complex_func1(8, 3);
            result2 = complex_func2(2.0, 5);
            result3 = complex_func3(7, 0);
            complex_func4(5, matrix);
            printf("Mode 0: %d, %.2f, %d\n", result1, result2, result3);
            break;
            
        case 1:
            result1 = complex_func1(15, 2);
            result2 = complex_func2(1.5, 8);
            result3 = complex_func3(4, 0);
            complex_func4(3, matrix);
            printf("Mode 1: %d, %.2f, %d\n", result1, result2, result3);
            break;
            
        case 2:
            result1 = complex_func1(3, 10);
            result2 = complex_func2(0.5, 4);
            result3 = complex_func3(10, 0);
            complex_func4(7, matrix);
            printf("Mode 2: %d, %.2f, %d\n", result1, result2, result3);
            break;
            
        case 3:
            /* Call all functions multiple times */
            for (volatile int k = 0; k < 2; ++k) {
                result1 = complex_func1(6 + k, 4);
                result2 = complex_func2(1.2, 3 + k);
                result3 = complex_func3(5, k);
                complex_func4(4 + k, matrix);
            }
            printf("Mode 3: %d, %.2f, %d\n", result1, result2, result3);
            break;
    }
    
    /* Additional execution for more coverage */
    if (mode > 10) {
        /* Uncommon path */
        result1 = complex_func1(20, 1);
        result2 = complex_func2(3.0, 2);
    }
    
    return 0;
}
