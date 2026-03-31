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
    }
    
    return result;
}

/* Function 2: Different complexity with recursion-like pattern */
int func2(volatile int n) {
    int total = 0;
    
    /* While loop */
    volatile int count = n;
    while (count > 0) {
        total += count;
        
        /* Multiple conditionals */
        if (count > 10) {
            total *= 3;
        } else if (count > 5) {
            total *= 2;
        }
        
        count--;
    }
    
    /* Do-while loop */
    volatile int j = 0;
    do {
        total -= j;
        j++;
    } while (j < 3);
    
    return total;
}

/* Function 3: Matrix-like operations */
int func3(volatile int rows, volatile int cols) {
    int sum = 0;
    
    /* Nested loops */
    for (volatile int r = 0; r < rows; ++r) {
        for (volatile int c = 0; c < cols; ++c) {
            sum += r * c;
            
            /* Early exit condition */
            if (sum > 1000) {
                goto done;
            }
        }
    }
    
done:
    /* Ternary operator */
    return (sum > 500) ? sum / 2 : sum * 2;
}

/* Function 4: String processing simulation */
int func4(volatile int mode) {
    int value = 0;
    
    /* Complex if-else chain */
    if (mode == 0) {
        value = 10;
    } else if (mode == 1) {
        value = 20;
        /* Small loop */
        for (volatile int k = 0; k < 3; ++k) {
            value += k;
        }
    } else if (mode == 2) {
        value = 30;
        /* Multiple increments */
        volatile int m = 0;
        while (m < 5) {
            value++;
            m++;
        }
    } else {
        value = 40;
    }
    
    /* Final adjustment */
    value = (value % 2 == 0) ? value + 1 : value - 1;
    
    return value;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 0;
    
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    /* Call functions with different arguments based on input */
    int r1 = func1(arg, arg % 3);
    int r2 = func2(arg + 2);
    int r3 = func3(arg % 4 + 1, arg % 3 + 1);
    int r4 = func4(arg % 4);
    
    /* Conditional based on results */
    if (r1 > r2) {
        printf("Result1: %d > %d\n", r1, r2);
    } else if (r1 < r2) {
        printf("Result1: %d < %d\n", r1, r2);
    } else {
        printf("Result1: %d == %d\n", r1, r2);
    }
    
    /* Another conditional */
    if ((r3 + r4) > 100) {
        printf("Sum large: %d\n", r3 + r4);
    } else {
        printf("Sum small: %d\n", r3 + r4);
    }
    
    return 0;
}
