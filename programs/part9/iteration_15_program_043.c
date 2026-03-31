/* Test program for gcov-tool overlap functionality */
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
    int sum = 0;
    for (i = 0; i < n; i++) {
        sum += i;
        if (i % 2 == 0) {
            global_counter++;
        } else {
            global_counter += 2;
        }
    }
    printf("func2: sum=%d\n", sum);
}

/* Function 3: Nested conditionals */
void func3(volatile int a, volatile int b) {
    if (a > b) {
        if (a > 10) {
            printf("func3: a=%d is large\n", a);
        } else {
            printf("func3: a=%d is moderate\n", a);
        }
    } else if (a < b) {
        printf("func3: a=%d < b=%d\n", a, b);
    } else {
        printf("func3: equal\n");
    }
}

/* Function 4: Complex control flow */
void func4(volatile int mode) {
    volatile int i = 0;
    switch (mode) {
        case 1:
            while (i < 5) {
                global_counter += i;
                i++;
            }
            break;
        case 2:
            do {
                global_counter -= i;
                i++;
            } while (i < 3);
            break;
        case 3:
            for (i = 0; i < 4; i++) {
                if (i == 2) continue;
                global_counter *= (i + 1);
            }
            break;
        default:
            global_counter = 0;
    }
    printf("func4: mode=%d, i=%d\n", mode, i);
}

int main(int argc, char *argv[]) {
    volatile int run_mode = 1;
    
    if (argc > 1) {
        run_mode = atoi(argv[1]);
    }
    
    printf("Running with mode=%d\n", run_mode);
    
    /* Call functions with different arguments based on run_mode */
    func1(run_mode);
    func2(run_mode + 2);
    func3(run_mode, run_mode * 2);
    func4(run_mode % 4);
    
    /* Additional calls for varied coverage */
    if (run_mode > 5) {
        func1(-run_mode);
        func2(1);
    }
    
    if (run_mode % 2 == 0) {
        func3(run_mode * 3, run_mode);
    }
    
    printf("Final counter: %d\n", global_counter);
    
    return 0;
}
