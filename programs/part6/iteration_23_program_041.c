/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that if-conversion would like to convert */
    if (x > 0) {
        result = y * 2;
        x = 0;  /* Modifies the condition variable x */
    } else {
        result = y / 2;
    }
    
    return result + x;  /* Use x to prevent dead store elimination */
}

int test1_modify_in_both(int x, int y) {
    int result;
    
    /* Both branches modify the condition variable */
    if (x != 0) {
        result = y + 10;
        x = x * 2;  /* Modifies condition in then */
    } else {
        result = y - 10;
        x = 5;      /* Modifies condition in else */
    }
    
    return result * x;
}

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int sum = 0;
    
    /* Use volatile to prevent constant folding */
    volatile int cond1 = seed;
    volatile int cond2 = seed + 1;
    
    sum += test1_modify_in_then(cond1, cond2);
    sum += test1_modify_in_both(cond2, cond1);
    
    printf("Test1 result: %d\n", sum);
    return sum > 100 ? 0 : 1;
}
