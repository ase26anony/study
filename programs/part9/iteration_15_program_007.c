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
        for (volatile int j = 0; j < y; ++j) {
            result += (i * j) / 3;
        }
    }
    
    /* Switch statement */
    switch (x % 4) {
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

/* Function 2: Different complexity pattern */
double func2(volatile int iterations) {
    double total = 0.0;
    volatile int counter = 0;
    
    /* While loop */
    while (counter < iterations) {
        total += 1.0 / (counter + 1);
        
        /* Conditional with else-if chain */
        if (counter < 5) {
            total *= 1.1;
        } else if (counter < 10) {
            total *= 1.05;
        } else {
            total *= 0.95;
        }
        
        counter++;
    }
    
    /* Do-while loop */
    do {
        total -= 0.5;
        counter--;
    } while (counter > 0);
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int value) {
    int result = value;
    
    if (depth <= 0) {
        return result;
    }
    
    /* Multiple conditionals */
    if (value > 100) {
        result = func3(depth - 1, value / 2);
    } else if (value > 50) {
        result = func3(depth - 1, value * 2);
    } else {
        result = func3(depth - 1, value + 10);
    }
    
    /* Another conditional */
    result = (result % 2 == 0) ? result + 1 : result - 1;
    
    return result;
}

/* Function 4: Simple utility function */
void func4(volatile int flag) {
    volatile int x = 0;
    
    /* Simple loop */
    for (int i = 0; i < 10; i++) {
        x += i;
    }
    
    /* Conditional based on flag */
    if (flag) {
        printf("Flag is set: %d\n", x);
    } else {
        printf("Flag not set: %d\n", x);
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    volatile int param1 = 5;
    volatile int param2 = 3;
    
    /* Parse command line argument if provided */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 1:
            printf("Mode 1: %d\n", func1(param1, param2));
            func2(8);
            func4(1);
            break;
            
        case 2:
            printf("Mode 2: %d\n", func1(param2, param1));
            func3(3, 75);
            func4(0);
            break;
            
        case 3:
            printf("Mode 3: %lf\n", func2(12));
            func3(2, 120);
            func1(4, 6);
            break;
            
        default:
            printf("Default mode\n");
            func1(2, 2);
            func2(3);
            func3(1, 50);
            func4(1);
            break;
    }
    
    /* Always execute some common path */
    volatile int common = func1(3, 3) + (int)func2(5);
    printf("Common result: %d\n", common);
    
    return 0;
}
