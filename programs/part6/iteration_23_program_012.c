/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that if-conversion likes */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* MODIFIES CONDITION VARIABLE - should trigger modified_in_p */
    } else {
        result = y / 2;
    }
    
    /* Use modified x to prevent dead store elimination */
    return result + x;
}

int test1_modify_in_both(int x, int y) {
    int result;
    
    if (x != 0) {
        result = y + 10;
        x = x + 1;  /* Modify in then block */
    } else {
        result = y - 10;
        x = x - 1;  /* Also modify in else block */
    }
    
    return result * x;
}

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int sum = 0;
    
    /* Call with volatile-derived values to prevent constant folding */
    sum += test1_modify_in_then(seed, seed + 1);
    sum += test1_modify_in_both(seed + 2, seed + 3);
    
    printf("Result: %d\n", sum);
    return sum;
}
