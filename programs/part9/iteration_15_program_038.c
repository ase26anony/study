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
    
    /* Conditional block */
    if (y > 10) {
        result += 100;
        for (volatile int j = 0; j < 3; ++j) {
            result += j * 2;
        }
    } else if (y > 0) {
        result += 50;
    } else {
        result -= 20;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int iterations) {
    double sum = 0.0;
    
    /* Multiple loops */
    for (volatile int i = 0; i < iterations; ++i) {
        sum += i * 1.5;
        
        /* Switch statement */
        switch (i % 4) {
            case 0:
                sum += 0.1;
                break;
            case 1:
                sum += 0.2;
                break;
            case 2:
                sum += 0.3;
                break;
            case 3:
                sum += 0.4;
                /* Fall through */
            default:
                sum -= 0.05;
        }
    }
    
    /* While loop */
    volatile int count = 5;
    while (count-- > 0) {
        sum /= 1.1;
    }
    
    return sum;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int base) {
    int total = base;
    
    if (depth <= 0) {
        return total;
    }
    
    /* Multiple conditionals */
    for (volatile int i = 0; i < depth; ++i) {
        if (i < depth / 2) {
            total += i * 3;
        } else {
            total -= i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < 2; ++j) {
            total += (i + j) % 3;
        }
    }
    
    /* Early return condition */
    if (total > 1000) {
        return total / 2;
    }
    
    return total * 2;
}

/* Function 4: Simple helper */
void func4(volatile int mode) {
    volatile int temp = 0;
    
    do {
        temp += mode;
        mode--;
    } while (mode > 0);
    
    /* Multiple exit points */
    if (temp > 50) {
        return;
    }
    
    /* Another loop */
    for (volatile int k = 0; k < 10; ++k) {
        temp += k;
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 1;
    
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    /* Call functions with different arguments based on input */
    int r1 = func1(arg, arg * 2);
    double r2 = func2(arg + 3);
    int r3 = func3(arg % 5, arg * 10);
    func4(arg);
    
    /* Different execution paths */
    if (arg % 3 == 0) {
        r1 = func1(r1 % 10, r3 % 20);
    } else if (arg % 3 == 1) {
        r2 = func2(r1 + r3);
    } else {
        func3(2, r1 + (int)r2);
    }
    
    /* Final conditional */
    volatile int final = r1 + (int)r2 + r3;
    if (final > 100) {
        printf("Result: %d (large)\n", final);
    } else {
        printf("Result: %d (small)\n", final);
    }
    
    return 0;
}
