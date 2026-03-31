/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_condition_in_then(int x, int y) {
    int result;
    int cond = x > 0;  // Condition variable
    
    if (cond) {
        // This modifies the condition variable inside the then block
        cond = 0;  // This should trigger modified_in_p
        result = y * 2;
    } else {
        result = y + 5;
    }
    
    return result + cond;  // Use cond to prevent optimization
}

int test1_simple_assignment(int a, int b) {
    int res;
    int flag = (a != 0);  // Condition variable
    
    if (flag) {
        flag = a + b;  // Modify condition variable
        res = b * 3;
    } else {
        res = b / 2;
    }
    
    return res + (flag != 0);
}

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    int x = seed % 10;
    int y = seed / 10;
    
    int sum = 0;
    sum += test1_modify_condition_in_then(x, y);
    sum += test1_simple_assignment(x, y);
    
    printf("Test1 result: %d\n", sum);
    return sum;
}
