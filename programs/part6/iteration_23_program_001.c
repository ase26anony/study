/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

volatile int external_seed = 0;

int test1_modify_condition_in_then(int x, int y) {
    int result;
    int cond = x > 0;  // condition variable
    
    if (cond) {
        // This modifies the condition variable 'cond'
        cond = 0;  // This should trigger modified_in_p
        result = y * 2;
    } else {
        result = y + 5;
    }
    
    // Use cond to prevent optimization
    return result + cond;
}

int test1_modify_condition_via_pointer(int x, int y) {
    int result;
    int cond = x;
    int *p = &cond;
    
    if (cond > 10) {
        *p = 5;  // Modifies cond through pointer
        result = y * 3;
    } else {
        result = y - 2;
    }
    
    return result + cond;
}

int main_test1(int argc, char **argv) {
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int x = seed % 20;
    int y = seed % 30;
    
    int sum = 0;
    sum += test1_modify_condition_in_then(x, y);
    sum += test1_modify_condition_via_pointer(x, y);
    
    // Prevent optimization
    external_seed = sum;
    return sum;
}
