/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_condition_in_then(int x, int y) {
    int result;
    int cond = x > 0;  // condition variable
    
    if (cond) {
        // This modifies the condition variable inside the then block
        cond = 0;  // This should trigger modified_in_p detection
        result = y * 2;
    } else {
        result = y + 5;
    }
    
    return result + cond;  // Use cond to prevent optimization
}

int test1_simple_if_else(int a, int b) {
    int output;
    
    // Classic if-conversion candidate pattern
    if (a > b) {
        a = a - 1;  // Modifies condition variable 'a'
        output = 100;
    } else {
        output = 200;
    }
    
    return output + a;  // Use 'a' to prevent dead store elimination
}

int main_test1(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    int x = seed % 10;
    int y = seed % 20;
    
    int result1 = test1_modify_condition_in_then(x, y);
    int result2 = test1_simple_if_else(x, y);
    
    return result1 + result2;
}
