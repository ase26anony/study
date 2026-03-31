/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test_modify_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that's attractive for if-conversion */
    if (x > 0) {
        result = y + 10;
        x = 5;  /* This modifies the condition variable x */
    } else {
        result = y - 10;
    }
    
    return result + x;  /* Use x to prevent dead store elimination */
}

int test_modify_in_both(int a, int b) {
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

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int x = seed % 10;
    int y = seed / 10;
    
    int sum = 0;
    sum += test_modify_in_then(x, y);
    sum += test_modify_in_both(x, y);
    
    /* Use volatile to prevent optimization */
    volatile int output = sum;
    printf("Result: %d\n", output);
    
    return 0;
}
