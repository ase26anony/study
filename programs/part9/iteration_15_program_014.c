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
    
    /* While loop with volatile */
    volatile int count = n;
    while (count > 0) {
        total += count;
        
        /* Multiple conditionals */
        if (count > 10) {
            total *= 3;
        } else if (count > 5) {
            total *= 2;
        }
        
        count--;
    }
    
    /* Early return under condition */
    if (n > 20) {
        return total * 2;
    }
    
    return total;
}

/* Function 3: Matrix-like operations */
int func3(volatile int rows, volatile int cols) {
    int sum = 0;
    
    /* Nested loops */
    for (volatile int r = 0; r < rows; ++r) {
        for (volatile int c = 0; c < cols; ++c) {
            sum += r * c;
            
            /* Conditional inside nested loop */
            if ((r + c) % 3 == 0) {
                sum -= 10;
            }
        }
    }
    
    return sum;
}

/* Function 4: String/array processing */
int func4(volatile int size) {
    int array[100];
    int result = 0;
    
    /* Initialize array */
    for (volatile int i = 0; i < size && i < 100; ++i) {
        array[i] = i * 2;
    }
    
    /* Process array with different paths */
    for (volatile int i = 0; i < size && i < 100; ++i) {
        if (array[i] > 50) {
            result += array[i] / 2;
        } else if (array[i] > 25) {
            result += array[i];
        } else {
            result += 1;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    /* Parse command line argument for different execution paths */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int total = 0;
    
    /* Execute different function combinations based on mode */
    switch (mode % 4) {
        case 0:
            total += func1(5, 1);
            total += func2(8);
            total += func3(3, 4);
            break;
        case 1:
            total += func1(3, 2);
            total += func4(15);
            total += func2(12);
            break;
        case 2:
            total += func3(4, 5);
            total += func4(20);
            total += func1(7, 0);
            break;
        case 3:
            total += func2(15);
            total += func3(2, 8);
            total += func4(10);
            total += func1(4, 3);
            break;
    }
    
    /* Additional conditional execution */
    if (mode > 10) {
        total += func1(10, 4);
        total += func4(30);
    }
    
    if (mode % 2 == 0) {
        total += func2(6);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
