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
    
    if (n <= 0) {
        return 1;
    }
    
    /* While loop */
    volatile int count = n;
    while (count > 0) {
        total += count;
        
        /* Nested if-else chain */
        if (count % 3 == 0) {
            total *= 3;
        } else if (count % 5 == 0) {
            total /= 2;
        } else {
            total += 10;
        }
        
        count--;
    }
    
    return total;
}

/* Function 3: Matrix-like operations */
int func3(volatile int rows, volatile int cols) {
    int sum = 0;
    
    /* Double nested loop */
    for (volatile int i = 0; i < rows; ++i) {
        for (volatile int j = 0; j < cols; ++j) {
            sum += i * j;
            
            /* Conditional inside inner loop */
            if ((i + j) % 2 == 0) {
                sum += 1000;
            } else {
                sum -= 500;
            }
        }
    }
    
    return sum;
}

/* Function 4: Mixed control flow */
int func4(volatile int mode) {
    int value = 0;
    
    /* Do-while loop */
    volatile int iterations = 5;
    do {
        value += iterations * 10;
        
        /* Complex conditional */
        if (mode == 1 && iterations % 2 == 0) {
            value += 50;
        } else if (mode == 2 || iterations < 3) {
            value -= 25;
        } else {
            value *= 2;
        }
        
        iterations--;
    } while (iterations > 0);
    
    /* Goto for additional complexity (rare but valid) */
    if (value > 1000) {
        goto large_value;
    }
    
    return value * 2;
    
large_value:
    return value / 2;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 0;
    
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    /* Call functions with different arguments based on input */
    int r1 = func1(arg, arg % 4);
    int r2 = func2(arg + 1);
    int r3 = func3(arg % 3 + 1, arg % 4 + 1);
    int r4 = func4(arg % 3);
    
    /* Final conditional based on all results */
    if ((r1 + r2 + r3 + r4) > 10000) {
        printf("Large result: %d\n", r1 + r2 + r3 + r4);
    } else if ((r1 + r2 + r3 + r4) < 0) {
        printf("Negative result: %d\n", r1 + r2 + r3 + r4);
    } else {
        printf("Normal result: %d\n", r1 + r2 + r3 + r4);
    }
    
    return 0;
}
