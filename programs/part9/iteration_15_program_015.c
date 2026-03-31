/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex with loops and conditionals */
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
    
    /* Another conditional */
    if (y > 10) {
        result += 100;
    } else if (y > 5) {
        result += 50;
    } else {
        result += 10;
    }
    
    return result;
}

/* Function 2: Different complexity with switch */
int func2(volatile int mode) {
    int total = 0;
    
    switch (mode) {
        case 1:
            total = 100;
            /* Fall through */
        case 2:
            total += 50;
            break;
        case 3:
            total = 200;
            /* Loop inside switch case */
            for (volatile int j = 0; j < 3; ++j) {
                total += j * 10;
            }
            break;
        default:
            total = -1;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth) {
    int value = depth;
    
    while (depth > 0) {
        if (value % 3 == 0) {
            value *= 2;
        } else if (value % 3 == 1) {
            value += 7;
        } else {
            value -= 5;
        }
        depth--;
    }
    
    return value;
}

/* Function 4: Simple helper */
int func4(volatile int a, volatile int b) {
    return a * b + (a > b ? a : b);
}

/* Main with different execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 1;
    
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    /* Different execution paths based on argument */
    int r1 = func1(arg, arg * 2);
    int r2 = func2(arg % 4);
    int r3 = func3(arg % 5 + 1);
    int r4 = func4(arg, arg + 1);
    
    /* Conditional that depends on results */
    if (r1 + r2 > r3 + r4) {
        printf("Path A: %d\n", r1 + r2);
    } else {
        printf("Path B: %d\n", r3 + r4);
    }
    
    /* Another loop for more coverage */
    volatile int final = 0;
    for (volatile int k = 0; k < arg % 10; ++k) {
        final += k * k;
    }
    
    printf("Final: %d\n", final);
    return 0;
}
