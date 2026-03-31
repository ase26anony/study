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

/* Function 2: Different complexity with recursion simulation */
int func2(volatile int n) {
    int total = 0;
    
    /* While loop with break */
    volatile int count = 0;
    while (count < n) {
        total += count;
        count++;
        
        if (total > 1000) {
            break;
        }
    }
    
    /* Do-while loop */
    volatile int j = 0;
    do {
        total -= j;
        j++;
    } while (j < 5);
    
    return total;
}

/* Function 3: Multiple nested conditionals */
int func3(volatile int a, volatile int b, volatile int c) {
    int val = 0;
    
    if (a > 0) {
        val += 10;
        if (b > 0) {
            val += 20;
            if (c > 0) {
                val += 30;
            } else {
                val += 40;
            }
        } else {
            val += 50;
        }
    } else {
        val += 60;
    }
    
    /* For loop with continue */
    for (volatile int k = 0; k < 10; ++k) {
        if (k % 3 == 0) {
            continue;
        }
        val += k;
    }
    
    return val;
}

/* Function 4: Simple arithmetic */
int func4(volatile int x) {
    return x * x + 2 * x + 1;
}

int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Execute different code paths based on mode */
    switch (mode) {
        case 0:
            result = func1(5, 1) + func2(3);
            break;
        case 1:
            result = func3(1, 1, 1) + func4(10);
            break;
        case 2:
            result = func1(3, 2) + func3(0, 1, 0);
            break;
        case 3:
            result = func2(10) + func4(5);
            break;
        default:
            result = func1(2, 0) + func2(2) + func3(1, 0, 1) + func4(3);
    }
    
    printf("Result: %d\n", result);
    
    /* Force more execution paths */
    if (mode % 2 == 0) {
        volatile int temp = func4(mode);
        result += temp;
    }
    
    return result % 256;
}
