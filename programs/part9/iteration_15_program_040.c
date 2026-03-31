#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    volatile int i;
    for (i = 0; i < x; ++i) {
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
int func2(volatile int n) {
    int total = 0;
    
    if (n <= 0) {
        return 1;
    }
    
    /* While loop */
    volatile int count = n;
    while (count > 0) {
        total += count;
        
        /* Multiple conditionals */
        if (count % 3 == 0) {
            total *= 3;
        } else if (count % 5 == 0) {
            total /= 2;
        } else {
            total += 7;
        }
        
        count--;
    }
    
    return total;
}

/* Function 3: Matrix-like operations */
int func3(volatile int rows, volatile int cols) {
    int sum = 0;
    
    /* Nested loops */
    volatile int r, c;
    for (r = 0; r < rows; ++r) {
        for (c = 0; c < cols; ++c) {
            sum += r * c;
            
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

/* Function 4: String processing simulation */
int func4(volatile int mode) {
    int value = 0;
    
    /* Do-while loop */
    volatile int iterations = 5;
    do {
        value += iterations * 10;
        
        /* Complex conditional chain */
        if (mode == 0) {
            value += 1;
        } else if (mode == 1) {
            value += 2;
            if (iterations % 2 == 0) {
                value *= 2;
            }
        } else if (mode == 2) {
            value += 3;
            /* Nested switch */
            switch (iterations) {
                case 1: value += 10; break;
                case 2: value += 20; break;
                case 3: value += 30; break;
                default: value += 5; break;
            }
        }
        
        iterations--;
    } while (iterations > 0);
    
    return value;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 0;
    
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    /* Call functions with different arguments based on input */
    int result1 = func1(arg, arg % 4);
    int result2 = func2(arg + 2);
    int result3 = func3(arg % 3 + 1, arg % 4 + 1);
    int result4 = func4(arg % 3);
    
    /* Conditional based on results */
    if (result1 > result2) {
        printf("Result1 dominates: %d\n", result1);
    } else if (result3 > result4) {
        printf("Result3 dominates: %d\n", result3);
    } else {
        printf("Mixed results: %d, %d, %d, %d\n", 
               result1, result2, result3, result4);
    }
    
    /* Final conditional to ensure different paths */
    if (arg % 2 == 0) {
        printf("Even argument path\n");
    } else {
        printf("Odd argument path\n");
    }
    
    return 0;
}
