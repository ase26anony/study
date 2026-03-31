/* Test program for gcov-tool overlap functionality */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: Simple function with conditional */
int func1(int x) {
    volatile int result = 0;
    if (x > 0) {
        result = x * 2;
    } else {
        result = x / 2;
    }
    
    /* Small loop */
    for (volatile int i = 0; i < 3; i++) {
        result += i;
    }
    
    return result;
}

/* Function 2: More complex with nested loops */
int func2(int x, int y) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (volatile int i = 0; i < x; i++) {
        /* Inner loop */
        for (volatile int j = 0; j < y; j++) {
            sum += i * j;
        }
        
        /* Conditional inside loop */
        if (i % 2 == 0) {
            sum += 100;
        } else {
            sum -= 50;
        }
    }
    
    return sum;
}

/* Function 3: Switch statement */
int func3(int option) {
    volatile int value = 0;
    
    switch (option) {
        case 1:
            value = 10;
            break;
        case 2:
            value = 20;
            /* Fall through */
        case 3:
            value += 5;
            break;
        default:
            value = -1;
    }
    
    /* While loop */
    volatile int count = 0;
    while (count < 2) {
        value += count * 2;
        count++;
    }
    
    return value;
}

/* Function 4: Recursive-like pattern */
int func4(int n) {
    volatile int total = 0;
    
    if (n <= 0) {
        return 1;
    }
    
    /* Multiple conditionals */
    if (n > 10) {
        total += 100;
    } else if (n > 5) {
        total += 50;
    } else {
        total += 10;
    }
    
    /* Do-while loop */
    volatile int i = 0;
    do {
        total += i;
        i++;
    } while (i < n && i < 4);
    
    return total;
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Execute different code paths based on mode */
    switch (mode) {
        case 1:
            /* Path 1: Call all functions lightly */
            func1(5);
            func2(2, 3);
            func3(1);
            func4(3);
            break;
            
        case 2:
            /* Path 2: Different arguments */
            func1(-3);
            func2(3, 2);
            func3(2);  /* This will fall through to case 3 */
            func4(8);
            break;
            
        case 3:
            /* Path 3: More intensive */
            for (volatile int i = 0; i < 2; i++) {
                func1(i * 10);
                func2(i + 1, i + 2);
            }
            func3(3);
            func4(12);
            break;
            
        default:
            /* Path 4: Default case */
            func1(mode);
            func2(1, 1);
            func3(99);
            func4(1);
    }
    
    /* Update global counter */
    global_counter += mode;
    
    return 0;
}
