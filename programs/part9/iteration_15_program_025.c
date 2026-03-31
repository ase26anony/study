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
            break;
    }
    
    return result;
}

/* Function 2: Different complexity with recursion-like pattern */
int func2(volatile int depth, volatile int base) {
    int total = base;
    
    /* While loop with volatile condition */
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
    /* Ternary operator */
    return (sum > 500) ? sum * 2 : sum / 2;
}

/* Function 4: String/array operations */
int func4(volatile int len) {
    int array[10];
    int result = 0;
    
    /* Fill array */
    for (volatile int i = 0; i < 10 && i < len; ++i) {
        array[i] = i * i;
    }
    
    /* Process array */
    for (volatile int i = 0; i < 10; ++i) {
        if (i < len) {
            result += array[i];
        } else {
            result -= i;
        }
        
        /* Multiple exit points */
        if (result < 0) {
            return -1;
        }
    }
    
    return result;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    /* Parse command line argument */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int total = 0;
    
    /* Execute different function combinations based on mode */
    switch (mode % 4) {
        case 0:
            total += func1(5, 1);
            total += func2(3, 10);
            total += func3(4);
            break;
        case 1:
            total += func1(3, 2);
            total += func4(8);
            total += func2(6, 5);
            break;
        case 2:
            total += func3(6);
            total += func4(5);
            total += func1(4, 3);
            break;
        case 3:
            total += func2(8, 2);
            total += func3(3);
            total += func4(10);
            break;
    }
    
    /* Additional conditional execution */
    if (mode > 10) {
        total += func1(10, 0);
    }
    
    if (mode % 2 == 0) {
        total += func2(4, 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
