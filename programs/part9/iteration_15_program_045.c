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
    
    /* Switch statement for complexity */
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

/* Function 2: Different complexity with recursion simulation */
int func2(volatile int n) {
    int total = 0;
    
    /* While loop with volatile */
    volatile int count = n;
    while (count > 0) {
        total += count;
        
        /* Multiple conditionals */
        if (count > 10) {
            total *= 2;
        } else if (count > 5) {
            total += 10;
        } else {
            total -= 5;
        }
        
        count--;
    }
    
    /* Do-while loop */
    volatile int j = 0;
    do {
        total += j * j;
        j++;
    } while (j < 3);
    
    return total;
}

/* Function 3: Matrix-like operations */
int func3(volatile int rows, volatile int cols) {
    int sum = 0;
    
    /* Nested loops */
    for (volatile int r = 0; r < rows; ++r) {
        for (volatile int c = 0; c < cols; ++c) {
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
    
    /* Complex if-else chain */
    if (mode == 0) {
        value = 10;
    } else if (mode == 1) {
        value = 20;
    } else if (mode == 2) {
        value = 30;
    } else if (mode == 3) {
        value = 40;
    } else {
        value = 50;
    }
    
    /* Loop with break */
    for (volatile int k = 0; k < 10; ++k) {
        value += k;
        if (value > 100) {
            break;
        }
    }
    
    return value;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int arg = 0;
    
    if (argc > 1) {
        arg = atoi(argv[1]);
    }
    
    int total = 0;
    
    /* Different execution paths based on argument */
    switch (arg % 4) {
        case 0:
            total += func1(5, 0);
            total += func2(8);
            total += func3(3, 4);
            total += func4(0);
            break;
        case 1:
            total += func1(3, 1);
            total += func2(12);
            total += func3(2, 2);
            total += func4(1);
            break;
        case 2:
            total += func1(7, 2);
            total += func2(5);
            total += func3(4, 3);
            total += func4(2);
            break;
        case 3:
            total += func1(4, 3);
            total += func2(15);
            total += func3(5, 5);
            total += func4(3);
            break;
    }
    
    /* Additional conditional execution */
    if (arg > 10) {
        total += func1(10, 4);
    }
    
    if (arg % 2 == 0) {
        total += func2(3);
    }
    
    printf("Total: %d\n", total);
    return 0;
}
