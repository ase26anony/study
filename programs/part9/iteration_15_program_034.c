#include <stdio.h>
#include <stdlib.h>

/* Function 1: Simple function with conditional */
int func1(int x) {
    volatile int result = 0;
    if (x > 0) {
        result = x * 2;
    } else {
        result = x / 2;
    }
    return result;
}

/* Function 2: Function with loop */
int func2(int n) {
    volatile int sum = 0;
    volatile int i;
    for (i = 0; i < n; i++) {
        sum += i;
        if (sum > 100) {
            sum = 100;  // Cap the sum
        }
    }
    return sum;
}

/* Function 3: Nested conditionals */
int func3(int a, int b) {
    volatile int val = 0;
    if (a > b) {
        val = a - b;
        if (val > 10) {
            val = 10;
        }
    } else if (a < b) {
        val = b - a;
        if (val > 20) {
            val = 20;
        }
    } else {
        val = 0;
    }
    return val;
}

/* Function 4: Complex function with switch */
int func4(int mode) {
    volatile int output = 0;
    switch (mode) {
        case 1:
            output = 100;
            break;
        case 2:
            output = 200;
            break;
        case 3:
            output = 300;
            break;
        default:
            output = -1;
    }
    
    // Additional loop
    volatile int j;
    for (j = 0; j < output % 10; j++) {
        output += j;
    }
    
    return output;
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int run_mode = 1;
    
    if (argc > 1) {
        run_mode = atoi(argv[1]);
    }
    
    // Call functions with different arguments based on run_mode
    int r1 = func1(run_mode);
    int r2 = func2(run_mode * 2);
    int r3 = func3(run_mode, run_mode * 2);
    int r4 = func4(run_mode % 4);
    
    // Create some branching based on results
    volatile int final_result = 0;
    if (r1 > r2) {
        final_result = r1 + r3;
    } else {
        final_result = r2 + r4;
    }
    
    // Another loop
    volatile int k;
    for (k = 0; k < (final_result % 5); k++) {
        final_result += k;
    }
    
    printf("Result: %d\n", final_result);
    return 0;
}
