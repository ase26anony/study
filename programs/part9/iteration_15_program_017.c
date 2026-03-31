/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with conditional */
    for (volatile int i = 0; i < x; ++i) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < y && j < 3; ++j) {
            result += j;
        }
    }
    
    /* Switch statement */
    switch (x % 4) {
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
            break;
    }
    
    return result;
}

/* Function 2: Different control flow pattern */
double func2(volatile int iterations) {
    double total = 0.0;
    volatile int i = 0;
    
    /* While loop */
    while (i < iterations) {
        if (i < iterations / 2) {
            total += i * 1.5;
        } else {
            total += i * 0.5;
        }
        
        /* Do-while loop */
        volatile int j = 0;
        do {
            total += 0.1;
            j++;
        } while (j < 2 && j < i);
        
        i++;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int value) {
    if (depth <= 0) {
        return value;
    }
    
    int result = value;
    
    /* Multiple conditionals */
    if (value > 10) {
        result += func3(depth - 1, value / 2);
    } else if (value > 5) {
        result += func3(depth - 1, value * 2);
    } else {
        result += func3(depth - 1, value + 1);
    }
    
    /* Ternary operator */
    return (depth % 2 == 0) ? result * 2 : result;
}

/* Function 4: Mixed control flow */
void func4(volatile int mode) {
    volatile int counter = 0;
    
    for (volatile int i = 0; i < 10; ++i) {
        switch (mode) {
            case 1:
                counter += i * i;
                break;
            case 2:
                counter += i * 2;
                /* Fall through */
            case 3:
                counter += 1;
                break;
            default:
                counter += i;
                break;
        }
        
        /* Early exit */
        if (counter > 50 && i > 5) {
            break;
        }
    }
    
    /* Multiple conditions */
    if (counter > 30 && mode != 2) {
        counter += 100;
    } else if (counter < 10 || mode == 1) {
        counter += 50;
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int run_mode = 1;
    
    if (argc > 1) {
        run_mode = atoi(argv[1]);
    }
    
    /* Execute different code paths based on run_mode */
    switch (run_mode) {
        case 1:
            printf("Mode 1: %d\n", func1(5, 3));
            printf("Func2: %.2f\n", func2(4));
            func4(1);
            break;
            
        case 2:
            printf("Mode 2: %d\n", func1(3, 5));
            printf("Func3: %d\n", func3(2, 8));
            func4(2);
            break;
            
        case 3:
            printf("Mode 3: %d\n", func1(7, 2));
            printf("Func2: %.2f\n", func2(6));
            printf("Func3: %d\n", func3(1, 15));
            func4(3);
            break;
            
        default:
            /* Execute all functions */
            printf("Default: %d\n", func1(4, 4));
            printf("Func2: %.2f\n", func2(3));
            printf("Func3: %d\n", func3(3, 6));
            func4(run_mode % 4);
            break;
    }
    
    return 0;
}
