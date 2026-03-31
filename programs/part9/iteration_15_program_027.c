/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with conditional inside */
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
    double sum = 0.0;
    
    /* While loop */
    volatile int count = 0;
    while (count < iterations) {
        if (count < iterations / 2) {
            sum += 1.5 * count;
        } else {
            sum += 2.5 * count;
        }
        
        /* Do-while inside while */
        volatile int inner = 0;
        do {
            sum += 0.1;
            inner++;
        } while (inner < 2);
        
        count++;
    }
    
    return sum;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int value) {
    if (depth <= 0) {
        return value;
    }
    
    int result = value;
    
    /* Multiple conditionals */
    if (value > 100) {
        result = func3(depth - 1, value / 2);
    } else if (value > 50) {
        result = func3(depth - 1, value - 10);
    } else {
        result = func3(depth - 1, value * 2);
    }
    
    /* Ternary operator */
    return (result % 2 == 0) ? result + 1 : result - 1;
}

/* Function 4: Simple utility */
void func4(volatile int flag) {
    static int counter = 0;
    
    if (flag) {
        counter += 10;
        printf("Counter increased: %d\n", counter);
    } else {
        counter -= 5;
        printf("Counter decreased: %d\n", counter);
    }
    
    /* Early return */
    if (counter < 0) {
        counter = 0;
        return;
    }
    
    /* Final processing */
    for (volatile int i = 0; i < 3; ++i) {
        counter += i;
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    volatile int param = 10;
    
    /* Parse command line for different execution paths */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        param = atoi(argv[2]);
    }
    
    printf("Running with mode=%d, param=%d\n", mode, param);
    
    /* Execute different function combinations based on mode */
    switch (mode) {
        case 1:
            printf("func1 result: %d\n", func1(param, 1));
            func4(1);
            break;
        case 2:
            printf("func2 result: %f\n", func2(param));
            printf("func3 result: %d\n", func3(3, param));
            break;
        case 3:
            printf("func1 result: %d\n", func1(param, 2));
            printf("func2 result: %f\n", func2(param / 2));
            func4(0);
            break;
        case 4:
            printf("func3 result: %d\n", func3(2, param));
            for (volatile int i = 0; i < 2; ++i) {
                func4(1);
            }
            break;
        default:
            /* Execute all functions */
            printf("func1 result: %d\n", func1(param, 3));
            printf("func2 result: %f\n", func2(param));
            printf("func3 result: %d\n", func3(1, param));
            func4(1);
    }
    
    return 0;
}
