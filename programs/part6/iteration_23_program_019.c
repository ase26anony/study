/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    /* Simple if-else pattern that if-conversion might consider */
    if (x > 0) {
        result = y + 10;
        x = 5;  /* Modifies the condition variable x */
    } else {
        result = y - 10;
    }
    return result + x;  /* Use x to prevent dead store elimination */
}

int test1_modify_in_both(int x, int y) {
    int result;
    /* Both branches modify the condition variable */
    if (x != 0) {
        result = y * 2;
        x = x + 1;  /* Modify condition in then */
    } else {
        result = y / 2;
        x = x - 1;  /* Modify condition in else */
    }
    return result + x;
}

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int x = seed;
    int y = seed * 2;
    
    int sum = 0;
    sum += test1_modify_in_then(x, y);
    sum += test1_modify_in_both(x, y);
    
    printf("Test1 result: %d\n", sum);
    return sum > 100 ? 0 : 1;
}
