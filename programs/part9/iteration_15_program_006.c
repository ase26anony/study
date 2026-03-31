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
        case 1:
            result += 100;
            break;
        case 2:
            result += 200;
            break;
        case 3:
            result += 300;
            break;
        default:
            result += 50;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int iterations) {
    double total = 0.0;
    volatile double factor = 1.5;
    
    /* While loop */
    volatile int count = 0;
    while (count < iterations) {
        total += count * factor;
        
        /* Multiple conditionals */
        if (count % 3 == 0) {
            total *= 1.1;
        } else if (count % 3 == 1) {
            total /= 1.05;
        } else {
            total -= 0.5;
        }
        
        count++;
    }
    
    /* Do-while loop */
    do {
        total += 10.0;
        count--;
    } while (count > 0);
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int value) {
    int result = value;
    
    /* Multiple nested conditionals */
    if (depth > 0) {
        for (volatile int i = 0; i < depth; i++) {
            if (i % 4 == 0) {
                result += func3(depth - 1, value + i);
            } else if (i % 4 == 1) {
                result -= i * 2;
            } else if (i % 4 == 2) {
                result *= (i + 1);
            } else {
                result /= (i > 0 ? i : 1);
            }
        }
    }
    
    /* Early return condition */
    if (result > 1000) {
        return result % 1000;
    }
    
    return result;
}

/* Function 4: Simple helper */
void func4(volatile int flag) {
    volatile static int counter = 0;
    
    if (flag) {
        for (volatile int i = 0; i < 5; i++) {
            counter += i;
            printf("Counter: %d\n", counter);
        }
    } else {
        counter = 0;
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    volatile int param1 = 3;
    volatile int param2 = 2;
    
    /* Different execution paths based on arguments */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            param1 = atoi(argv[2]);
        }
        if (argc > 3) {
            param2 = atoi(argv[3]);
        }
    }
    
    /* Call functions based on mode */
    switch (mode) {
        case 1:
            printf("Result1: %d\n", func1(param1, param2));
            func4(1);
            break;
        case 2:
            printf("Result2: %f\n", func2(param1));
            printf("Result3: %d\n", func3(param2, 10));
            break;
        case 3:
            printf("Result1: %d\n", func1(param2, param1));
            printf("Result2: %f\n", func2(param2));
            func4(0);
            break;
        default:
            printf("Default: %d, %f, %d\n", 
                   func1(2, 1), 
                   func2(3), 
                   func3(1, 5));
    }
    
    /* Always execute some common path */
    func4(1);
    
    return 0;
}
