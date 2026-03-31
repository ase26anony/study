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
            break;
    }
    
    return result;
}

/* Function 2: Different complexity with recursion-like pattern */
int func2(volatile int n) {
    int total = 0;
    
    while (n > 0) {
        total += n;
        
        /* Multiple conditionals */
        if (n > 10) {
            total *= 3;
        } else if (n > 5) {
            total *= 2;
        }
        
        /* Inner loop */
        for (volatile int j = 0; j < 3; ++j) {
            total += j;
            if (j == 1) {
                total -= 5;
            }
        }
        
        n--;
    }
    
    return total;
}

/* Function 3: Simple function for contrast */
int func3(volatile int a, volatile int b) {
    if (a > b) {
        return a * b;
    } else if (a < b) {
        return a + b;
    } else {
        return a - b;
    }
}

/* Function 4: More complex with goto for additional arcs */
int func4(volatile int mode) {
    int value = 0;
    
    if (mode == 0) {
        goto mode_zero;
    }
    
    /* Loop with break and continue */
    for (volatile int k = 0; k < 10; ++k) {
        if (k == 5) {
            break;
        }
        
        if (k % 2 == 0) {
            continue;
        }
        
        value += k * 2;
    }
    
    return value;

mode_zero:
    for (volatile int k = 0; k < 5; ++k) {
        value += k;
    }
    return value * 10;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 0;
    
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    /* Call functions with different arguments based on input */
    int r1 = func1(arg, arg % 4);
    int r2 = func2(arg + 2);
    int r3 = func3(arg, arg * 2);
    int r4 = func4(arg % 2);
    
    /* Create different execution paths */
    if (arg % 3 == 0) {
        printf("Path A: %d %d %d %d\n", r1, r2, r3, r4);
    } else if (arg % 3 == 1) {
        printf("Path B: %d %d\n", r1 + r2, r3 + r4);
    } else {
        printf("Path C: %d\n", r1 * r2 * r3 * r4);
    }
    
    return 0;
}
