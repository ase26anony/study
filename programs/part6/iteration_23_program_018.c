/* Test 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test_modify_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that if-conversion would like to convert */
    if (x > 0) {
        result = y * 2;  /* Simple operation */
        x = 5;           /* MODIFIES THE CONDITION VARIABLE - should trigger modified_in_p */
    } else {
        result = y + 10; /* Similar operation in else block */
    }
    
    /* Use modified x to prevent optimization */
    return result + x;
}

int test_modify_in_else(int x, int y) {
    int result;
    
    /* Test modification in else block instead */
    if (x <= 0) {
        result = y * 3;
    } else {
        result = y - 5;
        x = -1;          /* MODIFIES THE CONDITION VARIABLE in else block */
    }
    
    return result + x;
}

int main(int argc, char *argv[]) {
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int x = seed;
    int y = seed * 2 + 1;
    
    int sum = 0;
    sum += test_modify_in_then(x, y);
    sum += test_modify_in_else(x, y);
    
    printf("Result: %d\n", sum);
    return sum;
}
