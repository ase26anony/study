/* Test program for gcov-tool overlap functionality */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;

/* Function 1: Simple function with conditional */
void func1(int x) {
    volatile int local = x;
    if (local > 0) {
        global_counter += 1;
        printf("func1: positive %d\n", local);
    } else {
        global_counter -= 1;
        printf("func1: non-positive %d\n", local);
    }
}

/* Function 2: Function with loop */
void func2(int iterations) {
    volatile int i;
    volatile int sum = 0;
    
    for (i = 0; i < iterations; i++) {
        sum += i;
        if (i % 2 == 0) {
            global_counter += 2;
        } else {
            global_counter += 1;
        }
    }
    printf("func2: sum = %d\n", sum);
}

/* Function 3: Nested conditionals */
void func3(int a, int b) {
    volatile int result = 0;
    
    if (a > b) {
        result = a - b;
        if (result > 10) {
            global_counter *= 2;
        } else {
            global_counter += result;
        }
    } else if (a < b) {
        result = b - a;
        if (result < 5) {
            global_counter /= 2;
        }
    } else {
        global_counter = 0;
    }
    printf("func3: result = %d\n", result);
}

/* Function 4: Complex control flow */
void func4(int mode) {
    volatile int i, j;
    
    switch (mode) {
        case 0:
            for (i = 0; i < 3; i++) {
                global_counter += i;
            }
            break;
        case 1:
            i = 0;
            while (i < 4) {
                global_counter -= i;
                i++;
            }
            break;
        case 2:
            do {
                global_counter *= 2;
                j++;
            } while (j < 2);
            break;
        default:
            global_counter = 100;
    }
    printf("func4: mode %d, counter = %d\n", mode, global_counter);
}

/* Main function with different execution paths based on arguments */
int main(int argc, char *argv[]) {
    int run_mode = 0;
    
    if (argc > 1) {
        run_mode = atoi(argv[1]);
    }
    
    printf("Running with mode %d\n", run_mode);
    
    /* Different execution paths based on run_mode */
    switch (run_mode % 4) {
        case 0:
            func1(10);
            func2(3);
            func3(5, 2);
            func4(0);
            break;
        case 1:
            func1(-5);
            func2(5);
            func3(2, 8);
            func4(1);
            break;
        case 2:
            func1(0);
            func2(2);
            func3(10, 10);
            func4(2);
            break;
        case 3:
            func1(7);
            func2(4);
            func3(1, 20);
            func4(3);
            /* Call some functions twice for more coverage */
            func1(3);
            func2(1);
            break;
    }
    
    printf("Final counter: %d\n", global_counter);
    return 0;
}
