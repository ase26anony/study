/* Test 1: Integer condition modified in then block */
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

int test1_modify_in_else(int x, int y) {
    int result;
    /* Test modification in else block instead */
    if (x <= 0) {
        result = y * 2;
    } else {
        result = y / 2;
        x = -1;  /* Modifies condition variable in else block */
    }
    return result + x;
}

int main_test1(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int x = seed % 10;
    int y = seed % 20;
    
    int sum = 0;
    sum += test1_modify_in_then(x, y);
    sum += test1_modify_in_else(x, y);
    
    /* Use volatile to prevent optimization */
    volatile int output = sum;
    return output;
}
