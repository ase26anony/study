/* Test program for gcov-tool overlap functionality coverage */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: Simple function with conditional */
void func1(volatile int x) {
    if (x > 0) {
        global_counter += x;
        printf("func1: positive %d\n", x);
    } else {
        global_counter -= x;
        printf("func1: non-positive %d\n", x);
    }
}

/* Function 2: Function with loop */
void func2(volatile int n) {
    volatile int i;
    for (i = 0; i < n; i++) {
        global_counter++;
        if (i % 2 == 0) {
            printf("func2: even iteration %d\n", i);
        } else {
            printf("func2: odd iteration %d\n", i);
        }
    }
}

/* Function 3: Nested conditionals */
void func3(volatile int a, volatile int b) {
    if (a > b) {
        printf("func3: a > b\n");
        if (a > 10) {
            global_counter += 10;
        }
    } else if (a < b) {
        printf("func3: a < b\n");
        if (b > 10) {
            global_counter += 5;
        }
    } else {
        printf("func3: a == b\n");
        global_counter += 1;
    }
}

/* Function 4: Switch statement */
void func4(volatile int mode) {
    switch (mode) {
        case 0:
            printf("func4: mode 0\n");
            global_counter *= 2;
            break;
        case 1:
            printf("func4: mode 1\n");
            global_counter += 100;
            break;
        case 2:
            printf("func4: mode 2\n");
            global_counter -= 50;
            break;
        default:
            printf("func4: unknown mode\n");
            break;
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int run_mode = 0;
    
    if (argc > 1) {
        run_mode = atoi(argv[1]);
    }
    
    printf("Running with mode: %d\n", run_mode);
    
    /* Different execution paths based on run_mode */
    switch (run_mode) {
        case 0:
            func1(5);
            func2(3);
            func3(8, 12);
            func4(0);
            break;
        case 1:
            func1(-2);
            func2(5);
            func3(15, 10);
            func4(1);
            break;
        case 2:
            func1(0);
            func2(2);
            func3(7, 7);
            func4(2);
            break;
        default:
            func1(run_mode);
            func2(run_mode % 4);
            func3(run_mode, run_mode * 2);
            func4(3);
            break;
    }
    
    printf("Final counter: %d\n", global_counter);
    return 0;
}
