/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with conditional */
    for (volatile int i = 0; i < x; ++i) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < 3; ++j) {
            result += j;
        }
    }
    
    /* Switch statement */
    switch (y) {
        case 1:
            result += 10;
            break;
        case 2:
            result += 20;
            break;
        case 3:
            result += 30;
            break;
        default:
            result += 5;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int iterations) {
    double total = 0.0;
    
    /* While loop */
    volatile int count = 0;
    while (count < iterations) {
        total += count * 1.5;
        
        /* Conditional with else-if chain */
        if (count < 5) {
            total += 1.0;
        } else if (count < 10) {
            total += 2.0;
        } else {
            total += 3.0;
        }
        
        count++;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int max) {
    static int calls = 0;
    calls++;
    
    if (depth >= max) {
        return calls;
    }
    
    int sum = 0;
    for (volatile int i = 0; i < 2; ++i) {
        sum += func3(depth + 1, max);
    }
    
    return sum + depth;
}

/* Function 4: Simple hot/cold path function */
void func4(volatile int threshold) {
    volatile int hot_counter = 0;
    
    /* This loop creates hot paths */
    for (volatile int i = 0; i < 100; ++i) {
        if (i < threshold) {
            hot_counter += i * 2;  /* Hot path */
        } else {
            hot_counter += i;      /* Cold path */
        }
    }
    
    /* Another conditional */
    if (hot_counter > 1000) {
        printf("Hot path executed\n");
    }
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode: %d\n", mode);
    
    /* Execute different function combinations based on mode */
    switch (mode) {
        case 1:
            /* Execute all functions */
            printf("Result1: %d\n", func1(10, 2));
            printf("Result2: %f\n", func2(8));
            printf("Result3: %d\n", func3(0, 3));
            func4(50);
            break;
            
        case 2:
            /* Different execution pattern */
            printf("Result1: %d\n", func1(5, 3));
            printf("Result2: %f\n", func2(12));
            func4(30);
            break;
            
        case 3:
            /* Minimal execution */
            printf("Result1: %d\n", func1(3, 1));
            break;
            
        default:
            /* Error path */
            printf("Invalid mode\n");
            return 1;
    }
    
    return 0;
}
