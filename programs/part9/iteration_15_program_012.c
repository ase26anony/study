/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    volatile int i, j;
    
    /* Nested loops */
    for (i = 0; i < x; i++) {
        result += i;
        for (j = 0; j < y; j++) {
            result -= j;
            if (j % 2 == 0) {
                result *= 2;
            } else {
                result /= 2;
            }
        }
    }
    
    /* Conditional chain */
    if (x > 10) {
        result += 100;
    } else if (x > 5) {
        result += 50;
    } else {
        result += 10;
    }
    
    return result;
}

/* Function 2: Different complexity with switch */
int func2(volatile int mode) {
    int total = 0;
    volatile int counter = mode;
    
    switch (mode % 4) {
        case 0:
            total = mode * 2;
            break;
        case 1:
            total = mode + 100;
            break;
        case 2:
            total = mode - 50;
            break;
        case 3:
            total = mode / 2;
            break;
        default:
            total = -1;
    }
    
    /* While loop with break */
    while (counter > 0) {
        total += counter;
        if (counter % 7 == 0) {
            break;
        }
        counter--;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int base) {
    int acc = base;
    
    for (volatile int d = 0; d < depth; d++) {
        if (d % 3 == 0) {
            acc = acc * 2 + 1;
        } else if (d % 3 == 1) {
            acc = acc / 2 - 1;
        } else {
            acc = acc + d * 3;
        }
        
        /* Early return condition */
        if (acc > 1000) {
            return acc;
        }
    }
    
    /* Multiple exit points */
    if (acc < 0) {
        return 0;
    } else if (acc > 500) {
        return 500;
    }
    
    return acc;
}

/* Function 4: Simple utility */
int func4(volatile int a, volatile int b) {
    return a * b + (a - b);
}

int main(int argc, char *argv[]) {
    volatile int arg1 = 1;
    volatile int arg2 = 1;
    
    /* Different execution paths based on arguments */
    if (argc > 1) {
        arg1 = atoi(argv[1]);
    }
    if (argc > 2) {
        arg2 = atoi(argv[2]);
    }
    
    /* Call functions with different arguments to create varied coverage */
    int r1 = func1(arg1, arg2);
    int r2 = func2(arg1 + arg2);
    int r3 = func3(arg1 % 5 + 1, arg2);
    int r4 = func4(arg1, arg2);
    
    /* Use results to affect control flow */
    if ((r1 + r2 + r3 + r4) % 2 == 0) {
        printf("Even result: %d\n", r1 + r2);
    } else {
        printf("Odd result: %d\n", r3 + r4);
    }
    
    /* Additional call with different parameters */
    if (arg1 > arg2) {
        func1(arg2, arg1);
        func3(3, arg1 - arg2);
    }
    
    return 0;
}
