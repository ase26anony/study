/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test_modify_in_then(int x, int y) {
    int result;
    int cond = x > 0;  // Condition variable
    
    if (cond) {
        result = y * 2;
        cond = 0;  // MODIFIES the condition variable in then block
    } else {
        result = y / 2;
    }
    
    // Use cond again to prevent dead store elimination
    return result + (cond ? 1 : 0);
}

int test_modify_complex(int a, int b) {
    int temp = a + b;
    int flag = temp > 10;
    
    if (flag) {
        b = a * 2;
        flag = temp > 20;  // MODIFIES condition with different comparison
        a = b + 1;
    } else {
        b = a / 2;
    }
    
    return a + b + (flag ? 100 : 200);
}

int main(int argc, char **argv) {
    // Use argv to prevent constant folding
    int x = argc > 1 ? atoi(argv[1]) : 5;
    int y = argc > 2 ? atoi(argv[2]) : 3;
    
    int sum = 0;
    sum += test_modify_in_then(x, y);
    sum += test_modify_complex(x, y);
    
    // Also test with negative values
    sum += test_modify_in_then(-x, y);
    sum += test_modify_complex(x, -y);
    
    printf("Result: %d\n", sum);
    return sum;
}
