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

/* Function 2: Different complexity with recursion simulation */
int func2(volatile int depth, volatile int value) {
    int total = value;
    
    /* While loop */
    volatile int count = 0;
    while (count < depth) {
        total += count * 10;
        
        /* Multiple conditionals */
        if (total > 1000) {
            total /= 2;
        } else if (total > 500) {
            total -= 100;
        } else {
            total += 50;
        }
        
        count++;
    }
    
    /* Do-while loop */
    do {
        total -= 5;
        count--;
    } while (count > 0);
    
    return total;
}

/* Function 3: Matrix-like operations */
int func3(volatile int size) {
    int sum = 0;
    
    /* Nested loops */
    for (volatile int i = 0; i < size; ++i) {
        for (volatile int j = 0; j < size; ++j) {
            sum += i * j;
            
            /* Early exit condition */
            if (sum > 10000) {
                goto done;
            }
        }
    }
    
done:
    /* Conditional return */
    return (sum % 2 == 0) ? sum : sum + 1;
}

/* Function 4: String/array operations */
int func4(volatile int len) {
    int array[100];
    int total = 0;
    
    /* Initialize array */
    for (volatile int i = 0; i < len && i < 100; ++i) {
        array[i] = i * i;
    }
    
    /* Process array with different paths */
    for (volatile int i = 0; i < len && i < 100; ++i) {
        if (array[i] < 100) {
            total += array[i];
        } else if (array[i] < 1000) {
            total += array[i] / 2;
        } else {
            total += 10;
        }
        
        /* Complex condition */
        if (i > 10 && total % 3 == 0) {
            total -= array[i];
        }
    }
    
    return total;
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    volatile int param = 5;
    
    /* Parse command line for different execution modes */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        param = atoi(argv[2]);
    }
    
    int result = 0;
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 1:
            result = func1(param, 1) + func2(3, param);
            break;
        case 2:
            result = func3(param) + func4(param);
            break;
        case 3:
            result = func1(param, 2) + func3(param / 2);
            break;
        case 4:
            result = func2(param, 100) + func4(param * 2);
            break;
        default:
            result = func1(2, 3) + func2(2, 2) + func3(2) + func4(2);
    }
    
    /* Final conditional output */
    if (result > 1000) {
        printf("Large result: %d\n", result);
    } else {
        printf("Small result: %d\n", result);
    }
    
    return 0;
}
