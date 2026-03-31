/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    volatile int i;
    for (i = 0; i < x; i++) {
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

/* Function 2: Different complexity with switch */
int func2(volatile int mode) {
    int value = 0;
    
    switch (mode) {
        case 1:
            value = 100;
            /* Fall through */
        case 2:
            value += 50;
            break;
        case 3:
            value = 200;
            /* Loop inside switch case */
            volatile int j;
            for (j = 0; j < 3; j++) {
                value += j * 10;
            }
            break;
        default:
            value = -1;
    }
    
    return value;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth) {
    int total = 0;
    
    while (depth > 0) {
        total += depth;
        
        /* Inner conditional */
        if (depth % 3 == 0) {
            total *= 2;
        }
        
        depth--;
        
        /* Early exit condition */
        if (total > 1000) {
            break;
        }
    }
    
    return total;
}

/* Function 4: Simple helper */
int func4(volatile int a, volatile int b) {
    return a * b + (a > b ? a : b);
}

/* Main with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg_val = 1;
    
    if (argc > 1) {
        arg_val = atoi(argv[1]);
    }
    
    /* Different execution paths based on argument */
    int result1 = func1(arg_val, arg_val * 2);
    int result2 = func2(arg_val % 4);
    
    if (arg_val % 2 == 0) {
        result1 += func3(arg_val);
    } else {
        result2 += func4(arg_val, arg_val + 1);
    }
    
    /* Always execute some path */
    if (arg_val > 5) {
        printf("Result1: %d\n", result1);
    } else {
        printf("Result2: %d\n", result2);
    }
    
    /* Final call that always executes */
    func4(result1, result2);
    
    return 0;
}
