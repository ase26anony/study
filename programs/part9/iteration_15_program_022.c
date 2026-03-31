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

/* Function 2: Different complexity with recursion simulation */
int func2(volatile int n) {
    int total = 0;
    
    /* While loop */
    volatile int count = n;
    while (count > 0) {
        total += count;
        
        /* Multiple conditionals */
        if (count > 10) {
            total *= 3;
        } else if (count > 5) {
            total *= 2;
        } else {
            total += 1;
        }
        
        count--;
    }
    
    /* Do-while loop */
    volatile int j = 0;
    do {
        total -= j;
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

/* Function 4: String/array operations */
int func4(volatile int size) {
    int array[100];
    int total = 0;
    
    /* Initialize array */
    for (volatile int i = 0; i < size && i < 100; ++i) {
        array[i] = i * i;
    }
    
    /* Process array with multiple branches */
    for (volatile int i = 0; i < size && i < 100; ++i) {
        if (array[i] < 50) {
            total += 1;
        } else if (array[i] < 200) {
            total += 2;
        } else {
            total += 3;
        }
        
        /* Complex condition */
        if (i > 0 && (array[i] % 2 == 0 || array[i] % 3 == 0)) {
            total -= 1;
        }
    }
    
    return total;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 0:
            /* Path 1: Call all functions lightly */
            result += func1(3, 1);
            result += func2(4);
            result += func3(2, 3);
            result += func4(5);
            break;
            
        case 1:
            /* Path 2: Stress some functions */
            result += func1(10, 2);
            result += func2(8);
            result += func3(5, 5);
            result += func4(20);
            break;
            
        case 2:
            /* Path 3: Different combination */
            result += func1(5, 3);
            result += func2(12);
            result += func3(3, 4);
            result += func4(15);
            break;
            
        case 3:
            /* Path 4: Minimal execution */
            result += func1(1, 0);
            result += func2(1);
            break;
            
        default:
            /* Path 5: All heavy */
            result += func1(15, 4);
            result += func2(20);
            result += func3(10, 10);
            result += func4(50);
            break;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
