/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    volatile int i;
    for (i = 0; i < x; i++) {
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
    }
    
    return result;
}

/* Function 2: Different complexity with recursion-like pattern */
int func2(volatile int depth) {
    int total = 0;
    
    if (depth <= 0) {
        return 1;
    }
    
    /* Multiple conditionals */
    if (depth > 10) {
        total += 1000;
    } else if (depth > 5) {
        total += 500;
    } else {
        total += 100;
    }
    
    /* While loop */
    volatile int count = depth;
    while (count > 0) {
        total += count;
        count--;
        
        /* Inner if */
        if (count % 3 == 0) {
            total *= 2;
        }
    }
    
    return total;
}

/* Function 3: Matrix-like operations */
int func3(volatile int size) {
    int sum = 0;
    
    /* Nested loops */
    volatile int i, j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            sum += i * j;
            
            /* Conditional break */
            if (sum > 1000) {
                sum -= 500;
            }
        }
    }
    
    /* Do-while loop */
    volatile int k = 0;
    do {
        sum += k;
        k++;
        
        if (k > 5) {
            /* Early return */
            return sum * 2;
        }
    } while (k < 10);
    
    return sum;
}

/* Function 4: String processing simulation */
int func4(volatile int mode) {
    int value = 0;
    char* operations[] = {"add", "sub", "mul", "div"};
    
    /* Loop with break/continue */
    for (volatile int idx = 0; idx < 4; idx++) {
        if (mode == 0 && idx == 2) {
            continue;  /* Skip multiplication in mode 0 */
        }
        
        if (mode == 1 && idx == 3) {
            break;  /* Stop before division in mode 1 */
        }
        
        /* Complex switch */
        switch (idx) {
            case 0:
                value += 10;
                /* Fall through */
            case 1:
                value += 5;
                break;
            case 2:
                value *= 2;
                break;
            case 3:
                if (value != 0) {
                    value /= 2;
                }
                break;
        }
    }
    
    return value;
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int run_mode = 0;
    
    /* Parse command line argument */
    if (argc > 1) {
        run_mode = atoi(argv[1]);
    }
    
    int total = 0;
    
    /* Execute different function combinations based on run_mode */
    switch (run_mode % 4) {
        case 0:
            total += func1(5, 1);
            total += func2(3);
            total += func3(2);
            break;
        case 1:
            total += func1(3, 2);
            total += func3(4);
            total += func4(0);
            break;
        case 2:
            total += func2(7);
            total += func4(1);
            break;
        case 3:
            total += func1(4, 0);
            total += func2(2);
            total += func3(3);
            total += func4(2);
            break;
    }
    
    /* Additional conditional execution */
    if (run_mode > 10) {
        /* Extra execution for high run modes */
        for (volatile int extra = 0; extra < 3; extra++) {
            total += func1(extra + 1, extra);
        }
    }
    
    printf("Total result: %d\n", total);
    return 0;
}
