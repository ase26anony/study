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
    
    /* Another conditional */
    if (y > 0) {
        for (volatile int j = 0; j < y; ++j) {
            result += j * j;
        }
    } else {
        result = -result;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int n, volatile double factor) {
    double total = 1.0;
    
    /* While loop */
    volatile int count = n;
    while (count > 0) {
        total *= factor;
        
        /* Switch statement */
        switch (count % 3) {
            case 0:
                total += 0.5;
                break;
            case 1:
                total -= 0.2;
                break;
            case 2:
                total *= 1.1;
                break;
        }
        
        count--;
    }
    
    /* Conditional return */
    if (total > 100.0) {
        return total / 2.0;
    } else if (total < 0.0) {
        return 0.0;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int base) {
    int value = base;
    
    if (depth <= 0) {
        return value;
    }
    
    /* Multiple conditionals */
    for (volatile int d = 0; d < depth; ++d) {
        if (value % 2 == 0) {
            value = value / 2 + 1;
        } else {
            value = value * 3 + 1;
        }
        
        /* Early exit condition */
        if (value > 1000) {
            break;
        }
    }
    
    return value;
}

/* Function 4: Simple helper */
void func4(volatile int times) {
    volatile int sum = 0;
    for (volatile int i = 0; i < times; ++i) {
        sum += i * i;
        
        if (sum % 7 == 0) {
            printf(".");
        }
    }
}

/* Main with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    volatile double factor = 1.5;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    if (argc > 2) {
        factor = atof(argv[2]);
    }
    
    /* Different execution paths based on mode */
    switch (mode % 4) {
        case 0:
            printf("Result1: %d\n", func1(5, 3));
            printf("Result2: %f\n", func2(4, factor));
            break;
        case 1:
            printf("Result3: %d\n", func3(7, mode));
            func4(6);
            printf("\n");
            break;
        case 2:
            printf("Result1: %d\n", func1(3, 8));
            printf("Result3: %d\n", func3(3, 100));
            printf("Result2: %f\n", func2(6, factor * 0.8));
            break;
        case 3:
            func4(10);
            printf("\nResult1: %d\n", func1(7, -2));
            printf("Result2: %f\n", func2(2, factor * 2.0));
            break;
    }
    
    /* Always execute some common path */
    volatile int common = func1(2, 2) + func3(1, 10);
    printf("Common: %d\n", common);
    
    return 0;
}
