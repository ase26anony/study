/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Simple integer condition modified in then block */
int test1_modify_in_then(int x, int y) {
    int result;
    /* This should trigger modified_in_p check */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies condition variable */
    } else {
        result = y / 2;
    }
    return result + x;  /* Use x to prevent dead store elimination */
}

/* Function 2: Condition variable modified in else block */
int test1_modify_in_else(int a, int b) {
    int res;
    if (a < b) {
        res = a + b;
    } else {
        res = a - b;
        a = b;  /* Modifies condition variable in else block */
    }
    return res * a;  /* Use a to prevent optimization */
}

/* Function 3: Multiple modifications in both blocks */
int test1_complex_modification(int p, int q) {
    int output;
    volatile int cond = p;  /* Use volatile to prevent optimization */
    
    if (cond > q) {
        output = p * q;
        cond = q;  /* First modification */
        cond++;    /* Second modification */
    } else {
        output = p + q;
        cond = p - q;  /* Modification in else */
    }
    
    return output + cond;
}

/* Main driver */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <num1> <num2>\n", argv[0]);
        return 1;
    }
    
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    
    int sum = 0;
    sum += test1_modify_in_then(x, y);
    sum += test1_modify_in_else(x, y);
    sum += test1_complex_modification(x, y);
    
    printf("Result: %d\n", sum);
    return sum;
}
