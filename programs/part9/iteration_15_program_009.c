#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x) {
    int result = 0;
    
    /* Loop with volatile counter */
    for (volatile int i = 0; i < x; ++i) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < 3; ++j) {
            result += j;
        }
    }
    
    /* Switch statement */
    switch (x % 4) {
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

/* Function 2: Different complexity pattern */
int func2(volatile int y) {
    int total = 1;
    
    /* While loop */
    volatile int count = y;
    while (count > 0) {
        if (count % 3 == 0) {
            total *= 2;
        } else if (count % 3 == 1) {
            total *= 3;
        } else {
            total *= 4;
        }
        count--;
    }
    
    /* Do-while loop */
    volatile int k = 2;
    do {
        total += k;
        k--;
    } while (k > 0);
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int z) {
    int sum = 0;
    
    /* Multiple condition checks */
    if (z < 0) {
        return -1;
    } else if (z == 0) {
        return 0;
    } else if (z == 1) {
        return 1;
    }
    
    /* Complex conditional chain */
    for (volatile int m = 0; m < z; m++) {
        if (m < z / 2) {
            sum += m * 10;
        } else {
            sum += m * 5;
        }
        
        /* Early return possibility */
        if (sum > 1000) {
            return sum;
        }
    }
    
    return sum;
}

/* Function 4: Simple helper */
int func4(volatile int a, volatile int b) {
    return a * b + (a % (b + 1));
}

int main(int argc, char *argv[]) {
    volatile int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Execute different code paths based on mode */
    switch (mode % 4) {
        case 0:
            result = func1(5) + func2(3);
            break;
        case 1:
            result = func2(4) + func3(2);
            break;
        case 2:
            result = func1(3) + func4(7, 2);
            break;
        case 3:
            result = func3(4) + func4(3, 5);
            break;
        default:
            result = func1(2) + func2(2) + func3(2);
            break;
    }
    
    /* Additional conditional execution */
    if (mode > 10) {
        result += func4(mode, 2);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
