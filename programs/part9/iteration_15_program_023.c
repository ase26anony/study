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
    
    /* Another conditional block */
    if (y > 10) {
        for (volatile int j = 0; j < 3; ++j) {
            result += j * y;
        }
    } else if (y < 0) {
        result = -result;
    } else {
        result += 100;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int a, volatile double b) {
    double total = 0.0;
    
    /* While loop */
    volatile int count = a;
    while (count > 0) {
        total += b;
        
        /* Switch statement */
        switch (count % 3) {
            case 0:
                total *= 1.1;
                break;
            case 1:
                total /= 1.05;
                break;
            case 2:
                total -= 0.5;
                break;
        }
        
        count--;
    }
    
    /* Conditional at end */
    if (total > 50.0) {
        return total * 0.9;
    } else if (total < 10.0) {
        return total * 1.5;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int n, volatile int depth) {
    static int calls = 0;
    calls++;
    
    if (depth <= 0 || n <= 1) {
        return n;
    }
    
    int sum = 0;
    for (volatile int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            sum += func3(i, depth - 1);
        } else {
            sum -= i;
        }
    }
    
    /* Multiple exit points */
    if (sum > 1000) {
        return 1000;
    } else if (sum < -100) {
        return -100;
    }
    
    return sum;
}

/* Function 4: String processing */
void func4(volatile int mode) {
    char buffer[100];
    volatile int len = 0;
    
    /* Different paths based on mode */
    switch (mode) {
        case 1:
            for (volatile int i = 0; i < 10; ++i) {
                buffer[i] = 'A' + i;
                len++;
            }
            break;
        case 2:
            for (volatile int i = 0; i < 20; i += 2) {
                buffer[i/2] = 'Z' - i;
                len++;
            }
            break;
        case 3:
            len = 5;
            for (volatile int i = 0; i < len; ++i) {
                buffer[i] = '0' + i;
            }
            break;
        default:
            len = 1;
            buffer[0] = 'X';
            break;
    }
    
    /* Null terminate if we wrote anything */
    if (len > 0 && len < 100) {
        buffer[len] = '\0';
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 1;
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    /* Call functions with different arguments to create varied coverage */
    int r1 = func1(arg, arg * 2);
    double r2 = func2(arg, arg * 1.5);
    int r3 = func3(arg, 3);
    
    /* Different func4 calls based on argument */
    if (arg % 3 == 0) {
        func4(1);
    } else if (arg % 3 == 1) {
        func4(2);
    } else {
        func4(3);
    }
    
    /* Additional conditional execution */
    if (arg > 5) {
        r1 = func1(arg - 3, arg + 2);
        r2 = func2(arg / 2, r2);
    }
    
    /* Final computation */
    volatile int final = r1 + (int)r2 + r3;
    
    printf("Result: %d (arg=%d)\n", final, arg);
    return final % 100;
}
