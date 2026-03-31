/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    /* Simple if-else pattern that if-conversion would like to convert */
    if (x > 0) {
        result = y + 10;
        x = 5;  /* Modifies the condition variable x */
    } else {
        result = y - 10;
    }
    return result + x;  /* Use x to prevent dead store elimination */
}

int test1_modify_in_both(int a, int b) {
    int res;
    /* Both branches modify the condition variable */
    if (a != b) {
        res = a * 2;
        a = b;  /* Modify condition variable in then block */
    } else {
        res = b * 3;
        a = res;  /* Also modify in else block */
    }
    return res + a;
}

volatile int global_seed = 42;

int main(int argc, char **argv) {
    int sum = 0;
    
    /* Use volatile and argv to prevent constant folding */
    int x = global_seed;
    if (argc > 1) x = atoi(argv[1]);
    
    int y = (argc > 2) ? atoi(argv[2]) : 100;
    
    /* Call test functions with input-dependent values */
    sum += test1_modify_in_then(x, y);
    sum += test1_modify_in_both(x, y);
    
    printf("Test1 result: %d\n", sum);
    return sum;
}
