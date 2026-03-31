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

/* Function 2: Different complexity with recursion-like pattern */
int func2(volatile int depth, volatile int base) {
    int total = base;
    
    /* While loop with volatile */
    volatile int count = depth;
    while (count > 0) {
        total += count * 10;
        
        /* Multiple conditionals */
        if (count > 5) {
            total -= 25;
        } else if (count > 2) {
            total += 15;
        } else {
            total += 5;
        }
        
        count--;
    }
    
    /* Do-while loop */
    volatile int j = 0;
    do {
        total += j * 3;
        j++;
    } while (j < 3);
    
    return total;
}

/* Function 3: More complex with nested loops */
int func3(volatile int a, volatile int b) {
    int sum = 0;
    
    /* Nested loops */
    volatile int outer;
    for (outer = 0; outer < a; ++outer) {
        volatile int inner;
        for (inner = 0; inner < b; ++inner) {
            sum += outer * inner;
            
            /* Conditional inside nested loop */
            if ((outer + inner) % 3 == 0) {
                sum += 7;
            }
        }
        
        /* Break condition */
        if (sum > 1000) {
            sum -= 500;
        }
    }
    
    return sum;
}

/* Function 4: Simple function for contrast */
int func4(volatile int val) {
    if (val > 10) {
        return val * 2;
    } else if (val > 5) {
        return val + 10;
    } else {
        return val - 1;
    }
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    volatile int param1 = 3;
    volatile int param2 = 4;
    
    /* Parse command line for different execution paths */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        param1 = atoi(argv[2]);
    }
    if (argc > 3) {
        param2 = atoi(argv[3]);
    }
    
    int result = 0;
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 1:
            result = func1(param1, param2);
            result += func2(3, 10);
            break;
        case 2:
            result = func2(param1, param2);
            result += func3(2, 3);
            break;
        case 3:
            result = func3(param1, param2);
            result += func4(param1);
            break;
        case 4:
            result = func4(param1);
            result += func1(2, param2);
            break;
        default:
            result = func1(2, 2) + func2(2, 2) + func3(2, 2) + func4(2);
    }
    
    /* Force all functions to be called in some path */
    if (result > 1000) {
        /* Call remaining functions */
        volatile int temp = func4(result % 10);
        result += temp;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
