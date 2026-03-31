/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Direct modification of condition in then block */
int test_modify_in_then(int x, int y) {
    int result;
    /* This is the pattern we want: if (x > 0) { modify x } else { ... } */
    if (x > 0) {
        result = y * 2;
        x = 0;  /* Direct modification of condition variable in then block */
    } else {
        result = y / 2;
    }
    return result + x;  /* Use x to prevent dead store elimination */
}

/* Function 2: Modification through pointer in then block */
int test_modify_through_ptr(int x, int y) {
    int *ptr = &x;
    int result;
    
    if (x != 0) {
        result = y + 10;
        *ptr = 0;  /* Modify condition variable through pointer */
    } else {
        result = y - 10;
    }
    return result + x;
}

/* Function 3: Multiple modifications in then block */
int test_multiple_modifications(int x, int y) {
    int result;
    int original_x = x;
    
    if (x > 10 && x < 100) {
        result = y * 3;
        x = x + 1;      /* First modification */
        x = x * 2;      /* Second modification */
        x = x - 3;      /* Third modification */
    } else {
        result = y * 5;
    }
    
    /* Use both to prevent optimization */
    return result + (x != original_x);
}

int main(int argc, char *argv[]) {
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int x = seed + 5;
    int y = seed + 10;
    
    int sum = 0;
    sum += test_modify_in_then(x, y);
    sum += test_modify_through_ptr(x + 1, y + 1);
    sum += test_multiple_modifications(x + 2, y + 2);
    
    printf("Result: %d\n", sum);
    return sum;
}
