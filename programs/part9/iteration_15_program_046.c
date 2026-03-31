/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    for (volatile int i = 0; i < x; ++i) {
        result += i;
        
        /* Nested condition */
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
            /* Small loop */
            for (volatile int j = 0; j < 3; ++j) {
                value -= j;
            }
            break;
        default:
            value = -1;
    }
    
    /* Another conditional */
    if (value > 0) {
        volatile int k = 0;
        while (k < 2) {
            value *= 2;
            k++;
        }
    }
    
    return value;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth) {
    static int calls = 0;
    calls++;
    
    if (depth <= 0) {
        return calls;
    }
    
    int sum = 0;
    for (volatile int i = 0; i < depth; ++i) {
        sum += func3(depth - 1);
        
        /* Complex condition */
        if (sum > 1000) {
            sum /= 2;
        } else if (sum > 500) {
            sum -= 100;
        }
    }
    
    return sum;
}

/* Function 4: Simple helper */
int func4(volatile int a, volatile int b) {
    return a * b + (a > b ? a : b);
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 1;
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    int total = 0;
    
    /* Different execution paths based on argument */
    switch (arg) {
        case 1:
            total += func1(5, 3);
            total += func2(1);
            total += func3(2);
            total += func4(3, 4);
            break;
            
        case 2:
            total += func1(3, 12);
            total += func2(3);
            total += func3(1);
            total += func4(5, 2);
            break;
            
        case 3:
            total += func1(7, 7);
            total += func2(2);
            total += func3(3);
            total += func4(10, 1);
            break;
            
        default:
            total += func1(2, 2);
            total += func2(0);
            total += func3(0);
            total += func4(1, 1);
    }
    
    /* Final computation with loop */
    volatile int final = 0;
    for (volatile int i = 0; i < total % 10; ++i) {
        final += i * i;
    }
    
    printf("Result: %d\n", total + final);
    return 0;
}
