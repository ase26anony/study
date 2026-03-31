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

/* Function 2: Different complexity with recursion-like pattern */
int func2(volatile int n) {
    int total = 0;
    
    while (n > 0) {
        total += n;
        
        /* Multiple condition checks */
        if (n > 10) {
            total *= 3;
        } else if (n > 5) {
            total *= 2;
        }
        
        /* Inner loop */
        for (volatile int j = 0; j < 3; ++j) {
            total += j * n;
        }
        
        n--;
    }
    
    return total;
}

/* Function 3: Simple but with early returns */
int func3(volatile int a, volatile int b) {
    if (a == 0) {
        return b * 2;
    }
    
    if (b == 0) {
        return a * 3;
    }
    
    /* Nested loops */
    int sum = 0;
    for (volatile int i = 0; i < a; ++i) {
        for (volatile int j = 0; j < b; ++j) {
            sum += i * j;
            
            /* Conditional break */
            if (sum > 1000) {
                goto done;
            }
        }
    }
    
done:
    return sum;
}

/* Function 4: Utility function with array operations */
int func4(volatile int size) {
    if (size <= 0) return 0;
    
    int arr[10];
    int sum = 0;
    
    /* Fill array */
    for (volatile int i = 0; i < size && i < 10; ++i) {
        arr[i] = i * i;
    }
    
    /* Process array */
    for (volatile int i = 0; i < size && i < 10; ++i) {
        if (arr[i] % 2 == 0) {
            sum += arr[i];
        } else {
            sum -= arr[i] / 2;
        }
    }
    
    return sum;
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 1:
            result = func1(5, 2) + func2(3);
            break;
        case 2:
            result = func1(3, 1) + func3(4, 3);
            break;
        case 3:
            result = func2(7) + func4(8);
            break;
        case 4:
            result = func3(6, 2) + func4(5);
            break;
        default:
            result = func1(2, 3) + func2(2) + func3(2, 2) + func4(2);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
