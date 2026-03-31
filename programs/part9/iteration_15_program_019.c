/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    for (volatile int i = 0; i < x; ++i) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < y; ++j) {
            if (j < i) {
                result -= 1;
            } else {
                result += 1;
            }
        }
    }
    
    /* Switch statement */
    switch (result % 4) {
        case 0:
            result *= 2;
            break;
        case 1:
            result += 10;
            break;
        case 2:
            result -= 5;
            break;
        default:
            result = 0;
    }
    
    return result;
}

/* Function 2: Different control flow pattern */
double func2(volatile int iterations) {
    double total = 0.0;
    volatile int i = 0;
    
    /* While loop */
    while (i < iterations) {
        if (i < iterations / 2) {
            total += i * 1.5;
        } else {
            total -= i * 0.5;
        }
        
        /* Multiple conditions */
        if (i % 3 == 0 && i > 0) {
            total /= 2.0;
        } else if (i % 5 == 0) {
            total *= 1.1;
        }
        
        i++;
    }
    
    /* Do-while loop */
    do {
        total += 0.1;
        i--;
    } while (i > 0);
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int value) {
    if (depth <= 0) {
        return value;
    }
    
    int result = value;
    
    /* Multiple branches */
    if (value > 100) {
        for (volatile int k = 0; k < depth; ++k) {
            result -= k;
            if (k % 2 == 0) {
                result += func3(depth - 1, result / 2);
            }
        }
    } else if (value > 50) {
        volatile int m = 0;
        while (m < depth) {
            result += m * 3;
            m++;
            if (result > 200) break;
        }
    } else {
        result = func3(depth - 1, value * 2);
    }
    
    return result;
}

/* Function 4: Simple utility function */
void func4(volatile int count) {
    volatile int sum = 0;
    
    for (volatile int n = 0; n < count; ++n) {
        sum += n;
        
        /* Early return under condition */
        if (sum > 1000) {
            return;
        }
    }
    
    /* Multiple exit points */
    if (sum % 2 == 0) {
        printf("Even sum: %d\n", sum);
    } else {
        printf("Odd sum: %d\n", sum);
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode: %d\n", mode);
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 0:
            /* Path 1: Call all functions lightly */
            func1(5, 3);
            func2(4);
            func3(2, 30);
            func4(10);
            break;
            
        case 1:
            /* Path 2: Stress func1 and func2 */
            for (volatile int run = 0; run < 3; ++run) {
                func1(8, 4);
                func2(6);
            }
            func3(1, 100);
            break;
            
        case 2:
            /* Path 3: Deep recursion-like calls */
            func3(4, 25);
            func4(20);
            func1(3, 7);
            break;
            
        case 3:
            /* Path 4: Mixed intensive calls */
            func1(10, 5);
            func2(8);
            func3(3, 75);
            func4(15);
            func1(6, 2);
            break;
            
        default:
            /* Path 5: Minimal execution */
            func4(5);
            break;
    }
    
    return 0;
}
