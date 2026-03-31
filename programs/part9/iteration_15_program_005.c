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
            result += j;
            if (j > 5) {
                result -= 2;
            }
        }
    }
    
    /* Switch statement */
    switch (x % 4) {
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
            result /= 2;
            break;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int iterations) {
    double total = 0.0;
    volatile int count = iterations;
    
    while (count > 0) {
        if (count % 3 == 0) {
            total += 1.5;
        } else if (count % 3 == 1) {
            total += 2.5;
        } else {
            total += 0.5;
        }
        
        /* Early exit condition */
        if (total > 100.0) {
            break;
        }
        
        count--;
    }
    
    /* Do-while loop */
    volatile int k = 0;
    do {
        total += k * 0.1;
        k++;
    } while (k < 5);
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int max_depth) {
    static int calls = 0;
    calls++;
    
    if (depth >= max_depth) {
        return calls;
    }
    
    int sum = 0;
    for (volatile int i = 0; i < 3; ++i) {
        if (i == 0) {
            sum += func3(depth + 1, max_depth);
        } else if (i == 1) {
            sum += depth * 2;
        } else {
            sum += i;
        }
    }
    
    return sum;
}

/* Function 4: Hot/cold path simulation */
void func4(volatile int threshold) {
    volatile int hot_counter = 0;
    volatile int cold_counter = 0;
    
    for (volatile int i = 0; i < 100; ++i) {
        if (i < threshold) {
            /* Hot path */
            hot_counter += i * 2;
            if (hot_counter > 50) {
                hot_counter /= 2;
            }
        } else {
            /* Cold path */
            cold_counter += i;
            if (cold_counter % 7 == 0) {
                cold_counter -= 3;
            }
        }
    }
    
    /* Multiple return paths */
    if (hot_counter > cold_counter) {
        printf("Hot path dominant: %d > %d\n", hot_counter, cold_counter);
    } else if (cold_counter > hot_counter) {
        printf("Cold path dominant: %d > %d\n", cold_counter, hot_counter);
    } else {
        printf("Paths balanced: %d = %d\n", hot_counter, cold_counter);
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode: %d\n", mode);
    
    /* Execute different function combinations based on mode */
    switch (mode % 4) {
        case 0:
            printf("Result 1: %d\n", func1(10, 3));
            printf("Result 2: %.2f\n", func2(15));
            break;
        case 1:
            printf("Result 3: %d\n", func3(0, 2));
            func4(30);
            break;
        case 2:
            printf("Result 1: %d\n", func1(5, 8));
            printf("Result 3: %d\n", func3(0, 1));
            break;
        case 3:
            printf("Result 2: %.2f\n", func2(25));
            func4(70);
            printf("Result 1: %d\n", func1(7, 4));
            break;
    }
    
    /* Always execute some common path */
    volatile int common = func1(3, 2) + (int)func2(5);
    printf("Common result: %d\n", common);
    
    return 0;
}
