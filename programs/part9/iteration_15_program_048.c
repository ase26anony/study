/* test_overlap.c - Complex program to generate coverage data */
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
    
    /* Another loop */
    volatile int j = y;
    while (j > 0) {
        result += j;
        if (result > 100) {
            result = result % 100;
        }
        j--;
    }
    
    return result;
}

/* Function 2: Different control flow pattern */
double func2(volatile int n, volatile double factor) {
    double total = 0.0;
    
    /* Do-while loop */
    volatile int k = 0;
    do {
        total += k * factor;
        
        /* Switch statement */
        switch (k % 3) {
            case 0:
                total += 1.5;
                break;
            case 1:
                total *= 1.1;
                break;
            case 2:
                total -= 0.5;
                break;
        }
        k++;
    } while (k < n);
    
    /* Conditional chain */
    if (total < 0) {
        total = 0;
    } else if (total < 50) {
        total *= 2;
    } else {
        total /= 2;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern with goto */
int func3(volatile int depth, volatile int seed) {
    int value = seed;
    
    if (depth <= 0) {
        return value;
    }
    
    /* Use goto for unusual control flow */
    int counter = 0;
    start_loop:
    if (counter < depth) {
        value += (counter * 3);
        
        /* Nested if-else */
        if (value % 2 == 0) {
            goto even_case;
        } else {
            goto odd_case;
        }
        
        even_case:
        value /= 2;
        counter++;
        goto start_loop;
        
        odd_case:
        value *= 3;
        counter++;
        goto start_loop;
    }
    
    return value;
}

/* Function 4: Simple helper function */
void helper(volatile int *arr, volatile int size) {
    for (volatile int i = 0; i < size; i++) {
        arr[i] = i * i;
        
        /* Early return condition */
        if (arr[i] > 100) {
            return;
        }
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    volatile int loop1 = 5;
    volatile int loop2 = 3;
    volatile double factor = 1.5;
    
    /* Execute different code paths based on mode */
    switch (mode % 4) {
        case 0:
            printf("Result1: %d\n", func1(loop1, loop2));
            break;
        case 1:
            printf("Result2: %f\n", func2(loop1, factor));
            break;
        case 2:
            printf("Result3: %d\n", func3(loop1, loop2));
            break;
        case 3:
            {
                volatile int arr[10];
                helper(arr, 10);
                printf("Helper executed\n");
            }
            break;
    }
    
    /* Always execute some common code */
    volatile int common = 0;
    for (volatile int i = 0; i < 3; i++) {
        common += func1(i, i+1);
    }
    
    return 0;
}
