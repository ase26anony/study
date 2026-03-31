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
        
        /* Multiple conditionals */
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

/* Function 3: Simple function with early returns */
int func3(volatile int a, volatile int b) {
    if (a < 0) {
        return -1;
    }
    
    if (b < 0) {
        return -2;
    }
    
    /* Do-while loop */
    volatile int i = 0;
    do {
        a += b;
        i++;
    } while (i < 5);
    
    return a * b;
}

/* Function 4: Function with goto for additional complexity */
int func4(volatile int mode) {
    int value = 0;
    
    if (mode == 0) {
        goto mode_zero;
    }
    
    /* Complex nested loops */
    for (volatile int i = 0; i < 3; ++i) {
        for (volatile int j = 0; j < 3; ++j) {
            value += i * j;
            
            if (value > 10) {
                goto done;
            }
        }
    }
    
    return value;

mode_zero:
    value = 999;
    return value;

done:
    return value * 2;
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
    int r3 = func3(arg, arg + 2);
    int r4 = func4(arg % 3);
    
    /* Conditional execution based on results */
    if (r1 > r2) {
        printf("Result1: %d\n", r1);
    } else if (r3 > r4) {
        printf("Result2: %d\n", r2);
    } else {
        printf("Result3: %d, %d\n", r3, r4);
    }
    
    /* Additional execution path for more coverage */
    if (arg % 2 == 0) {
        /* Call functions again with different parameters */
        func1(arg * 2, 0);
        func2(arg / 2);
    } else {
        func3(arg + 10, arg - 5);
        func4(1);
    }
    
    return 0;
}
