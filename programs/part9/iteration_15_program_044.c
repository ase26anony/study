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
    
    /* Conditional based on y */
    if (y > 10) {
        result += 100;
    } else if (y > 5) {
        result += 50;
    } else {
        result += 10;
    }
    
    return result;
}

/* Function 2: Different complexity with switch statement */
int func2(volatile int mode) {
    int total = 0;
    
    switch (mode) {
        case 0:
            total = 100;
            break;
        case 1:
            total = 200;
            /* Fall through */
        case 2:
            total += 50;
            break;
        default:
            total = 0;
    }
    
    /* Another loop */
    for (volatile int j = 0; j < mode; ++j) {
        total += j * j;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth) {
    int value = depth;
    
    while (value > 0) {
        if (value % 3 == 0) {
            value /= 3;
        } else if (value % 2 == 0) {
            value /= 2;
        } else {
            value -= 1;
        }
    }
    
    return value;
}

/* Function 4: Matrix-like operation */
int func4(volatile int size) {
    int sum = 0;
    
    /* Nested loops */
    for (volatile int i = 0; i < size; ++i) {
        for (volatile int j = 0; j < size; ++j) {
            sum += i * j;
            
            /* Early exit condition */
            if (sum > 1000) {
                goto done;
            }
        }
    }
    
done:
    return sum;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 0;
    
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    /* Call functions with different arguments based on input */
    int r1 = func1(arg, arg * 2);
    int r2 = func2(arg % 4);
    int r3 = func3(arg);
    int r4 = func4(arg % 5 + 1);
    
    /* Conditional based on results */
    if (r1 > r2 && r3 < r4) {
        printf("Path A: %d %d %d %d\n", r1, r2, r3, r4);
    } else if (r1 + r2 > r3 + r4) {
        printf("Path B: %d %d %d %d\n", r1, r2, r3, r4);
    } else {
        printf("Path C: %d %d %d %d\n", r1, r2, r3, r4);
    }
    
    return 0;
}
