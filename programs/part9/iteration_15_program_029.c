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
int func2(volatile int depth, volatile int value) {
    int total = value;
    
    /* While loop */
    volatile int count = depth;
    while (count > 0) {
        total += count;
        
        /* Multiple conditionals */
        if (count > 5) {
            total *= 2;
        } else if (count > 2) {
            total += 10;
        } else {
            total -= 5;
        }
        
        count--;
    }
    
    /* Do-while loop */
    volatile int j = 0;
    do {
        total += j * j;
        j++;
    } while (j < 3);
    
    return total;
}

/* Function 3: Matrix-like operations */
int func3(volatile int size) {
    int sum = 0;
    
    /* Nested loops */
    for (volatile int i = 0; i < size; i++) {
        for (volatile int j = 0; j < size; j++) {
            sum += i * j;
            
            /* Early exit condition */
            if (sum > 1000) {
                goto done;
            }
        }
    }
    
done:
    /* Ternary operator */
    return (sum > 500) ? sum * 2 : sum / 2;
}

/* Function 4: Simple helper */
int func4(volatile int a, volatile int b) {
    return a * b + (a > b ? a : b);
}

/* Main with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Different execution paths based on mode */
    switch (mode % 4) {
        case 0:
            result = func1(5, 1) + func2(3, 10);
            break;
        case 1:
            result = func2(7, 5) + func3(4);
            break;
        case 2:
            result = func1(3, 2) + func3(3) + func4(6, 8);
            break;
        case 3:
            result = func2(4, 3) + func4(10, 5);
            break;
    }
    
    /* Additional conditional execution */
    if (mode > 10) {
        result += func1(10, 3);
    }
    
    if (mode % 2 == 0) {
        result += func3(2);
    } else {
        result += func4(2, 3);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
