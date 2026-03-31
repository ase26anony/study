/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_condition_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that's a candidate for if-conversion */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies the condition variable x */
    } else {
        result = y / 2;
    }
    
    /* Use x again to prevent dead store elimination */
    return result + x;
}

int test1_modify_condition_in_both(int x, int y) {
    int result;
    
    /* Condition variable modified in both branches */
    if (x != 0) {
        result = y + 10;
        x = x + 1;  /* Modify condition in then */
    } else {
        result = y - 10;
        x = x - 1;  /* Modify condition in else */
    }
    
    return result * x;
}

int main(int argc, char **argv) {
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int x = seed;
    int y = seed * 2;
    
    int sum = 0;
    sum += test1_modify_condition_in_then(x, y);
    sum += test1_modify_condition_in_both(x, y);
    
    printf("Test1 result: %d\n", sum);
    return sum;
}
