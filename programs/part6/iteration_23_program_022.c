/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 0;

int test1_modify_in_then(int x, int y) {
    int result;
    /* Simple if-else pattern that if-conversion would consider */
    if (x > 0) {
        /* This modifies the condition variable x inside the then block */
        x = y + 1;  // This should trigger modified_in_p
        result = 10;
    } else {
        result = 20;
    }
    /* Use x to prevent dead store elimination */
    return result + (x & 1);
}

int test1_modify_in_both(int a, int b) {
    int res;
    /* Condition variable modified in both branches */
    if (a == b) {
        a = 5;  // Modify in then
        res = 100;
    } else {
        a = 10; // Modify in else  
        res = 200;
    }
    return res + a;
}

int main(int argc, char **argv) {
    volatile int input1, input2;
    
    /* Use argv to get non-constant values */
    input1 = argc > 1 ? atoi(argv[1]) : 5;
    input2 = argc > 2 ? atoi(argv[2]) : 3;
    
    int sum = 0;
    sum += test1_modify_in_then(input1, input2);
    sum += test1_modify_in_both(input1, input2);
    
    /* Also test with global variable */
    global_seed = input1;
    if (global_seed > 10) {
        global_seed = 0;  // Modify global condition
        sum += 30;
    } else {
        sum += 40;
    }
    
    printf("Test1 result: %d\n", sum);
    return sum;
}
