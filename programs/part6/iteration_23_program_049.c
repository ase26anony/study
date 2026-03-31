/* Test 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test_modify_int_condition(int x, int y) {
    int result;
    // Condition variable x gets modified in then block
    if (x > 0) {
        result = y * 2;
        x = 0;  // Modifies the condition variable
    } else {
        result = y / 2;
    }
    return result + x;  // Use x to prevent dead store elimination
}

int test_modify_int_condition2(int a, int b) {
    int res;
    // Different modification pattern
    if (a != b) {
        res = a + b;
        a = b;  // Makes condition false if re-evaluated
    } else {
        res = a - b;
    }
    return res * a;  // Use modified a
}

int main(int argc, char **argv) {
    // Use argv to prevent constant folding
    int x = argc > 1 ? atoi(argv[1]) : 5;
    int y = argc > 2 ? atoi(argv[2]) : 10;
    
    int sum = 0;
    sum += test_modify_int_condition(x, y);
    sum += test_modify_int_condition2(x, y);
    
    // Also test with volatile to force reads
    volatile int v = 7;
    sum += test_modify_int_condition(v, y);
    
    printf("Result: %d\n", sum);
    return sum;
}
