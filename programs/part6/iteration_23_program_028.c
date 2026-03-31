/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_condition_in_then(int x, int y) {
    int result;
    int cond = x > 0;  // Condition variable
    
    if (cond) {
        result = y * 2;
        cond = 0;  // MODIFIES condition variable in then block
    } else {
        result = y / 2;
    }
    
    // Use result to prevent optimization
    return result + (cond ? 1 : 0);
}

int test1_pointer_arithmetic(int *ptr, int threshold) {
    int *cond_ptr = ptr;
    int result;
    
    if (cond_ptr != NULL) {
        result = *cond_ptr * 3;
        cond_ptr = NULL;  // MODIFIES pointer condition in then block
    } else {
        result = threshold;
    }
    
    return result + (cond_ptr == NULL ? 0 : 1);
}

int main_test1(int argc, char **argv) {
    int x = argc > 1 ? atoi(argv[1]) : 5;
    int y = argc > 2 ? atoi(argv[2]) : 10;
    
    int sum = 0;
    sum += test1_modify_condition_in_then(x, y);
    
    int arr[3] = {1, 2, 3};
    int *ptr = (x > 0) ? &arr[0] : NULL;
    sum += test1_pointer_arithmetic(ptr, y);
    
    return sum;
}
