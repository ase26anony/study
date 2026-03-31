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

/* Function 2: Different complexity with recursion simulation */
int func2(volatile int depth, volatile int value) {
    if (depth <= 0) {
        return value;
    }
    
    int sum = 0;
    for (volatile int i = 0; i < depth; ++i) {
        sum += value + i;
        
        /* Multiple conditionals */
        if (sum > 1000) {
            sum /= 2;
        } else if (sum > 500) {
            sum -= 100;
        } else {
            sum += 50;
        }
    }
    
    /* Call another function */
    return sum + func1(depth, value % 3);
}

/* Function 3: Matrix-like operations */
int func3(volatile int size) {
    int total = 0;
    
    /* Double nested loop */
    for (volatile int i = 0; i < size; ++i) {
        for (volatile int j = 0; j < size; ++j) {
            total += i * j;
            
            /* Conditional inside inner loop */
            if ((i + j) % 3 == 0) {
                total -= j;
            } else if ((i + j) % 3 == 1) {
                total += i;
            }
        }
    }
    
    return total;
}

/* Function 4: String processing simulation */
int func4(volatile int mode) {
    int count = 0;
    volatile int limit = 10;
    
    /* While loop */
    while (limit > 0) {
        count += mode;
        
        /* Complex conditional chain */
        if (mode == 1) {
            count *= 2;
        } else if (mode == 2) {
            count += limit;
        } else if (mode == 3) {
            count -= limit / 2;
        } else {
            count = count > 100 ? count - 50 : count + 25;
        }
        
        limit--;
    }
    
    return count;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Execute different code paths based on mode */
    switch (mode % 4) {
        case 0:
            result = func1(5, 1) + func2(3, 10);
            printf("Mode 0: %d\n", result);
            break;
        case 1:
            result = func2(4, 5) + func3(3);
            printf("Mode 1: %d\n", result);
            break;
        case 2:
            result = func3(4) + func4(2);
            printf("Mode 2: %d\n", result);
            break;
        case 3:
            result = func1(3, 2) + func4(1) + func2(2, 8);
            printf("Mode 3: %d\n", result);
            break;
    }
    
    /* Additional execution for more coverage */
    if (mode > 10) {
        /* Rare path */
        result += func1(10, 0) * func2(5, 2);
    }
    
    return result % 1000;
}
