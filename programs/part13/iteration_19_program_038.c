/* Test program for GCC auto-profile coverage of phi-node-defined conditional branches */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization of copy chains */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Function with loop-carried dependency forming phi */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for 'flag' at loop header */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* 'flag' is defined by phi node at start of this block */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
    /* Critical conditional: compare phi-defined variable against 0 */
    if (flag_copy == 0) {  /* cmp_rhs is integer constant 0 */
        result += 100;
    } else {
        result += 200;
    }
    
    return result;
}

/* Function with multiple returns creating phi for return value */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    if (x > y) {
        condition = 1;  /* One incoming edge */
    } else if (x < y) {
        condition = 0;  /* Another incoming edge */
    } else {
        condition = 1;  /* Third incoming edge */
    }
    
    /* condition is defined by phi node here */
    int a = condition;
    int b = a;
    int c = b;  /* Copy chain */
    
    /* Compare against constant 1 */
    if (c == 1) {  /* cmp_rhs is integer constant 1 */
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Function with switch statement creating phi */
int test_phi_from_switch(int mode) {
    int status = 0;
    
    switch (mode % 4) {
        case 0:
            status = 1;  /* One incoming value */
            break;
        case 1:
            status = 0;  /* Another incoming value */
            break;
        case 2:
            status = 1;  /* Third incoming value */
            break;
        case 3:
            status = 0;  /* Fourth incoming value */
            break;
    }
    
    /* Create longer copy chain */
    int tmp1 = status;
    volatile_sink = tmp1;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    
    /* Compare against 0 with != operator */
    if (tmp3 != 0) {  /* Still comparing against constant 0 */
        return 1000;
    }
    return 2000;
}

/* Function with boolean variable (guaranteed 0/1) */
bool test_phi_boolean(int a, int b) {
    bool greater;
    
    /* Multiple paths setting boolean */
    if (a > 100) {
        greater = true;  /* true = 1 */
    } else if (b > 100) {
        greater = false; /* false = 0 */
    } else {
        greater = (a > b);
    }
    
    /* Boolean is 0/1, defined by phi */
    bool copy1 = greater;
    bool copy2 = copy1;
    
    /* Implicit comparison against 0 in boolean context */
    if (copy2) {  /* Equivalent to 'if (copy2 != 0)' */
        return true;
    }
    return false;
}

/* Recursive function creating phi for condition */
int test_phi_recursive(int n, int depth) {
    if (depth <= 0) {
        return n % 2;  /* Returns 0 or 1 */
    }
    
    int left = test_phi_recursive(n * 3 + 1, depth - 1);
    int right = test_phi_recursive(n * 2, depth - 1);
    
    /* 'result' gets phi from two recursive calls */
    int result = (left > right) ? left : right;
    
    /* Copy chain */
    int r1 = result;
    int r2 = r1;
    
    /* Compare against 1 */
    if (r2 == 1) {
        return n + 1;
    }
    return n - 1;
}

/* Function with hot path using __builtin_expect */
int test_hot_path_annotation(int x) {
    int state = 0;
    
    /* Force this to be hot path */
    if (__builtin_expect(x > 0, 1)) {
        state = 1;
    } else {
        state = 0;
    }
    
    /* Multiple copy assignments */
    int s1 = state;
    volatile_sink = s1;
    int s2 = s1;
    int s3 = s2;
    
    /* Critical conditional on hot path */
    if (s3 == 1) {  /* Compare against 1 */
        return x * 10;
    }
    return x * 5;
}

/* Main function to create hot execution paths */
int main(void) {
    int total = 0;
    
    /* Execute many times to make paths hot */
    for (int i = 0; i < 100000; ++i) {
        /* Mix different patterns */
        total += test_phi_after_loop(i % 100);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        total += test_phi_boolean(i, i * 2);
        
        if (i % 1000 == 0) {  /* Less frequent recursive calls */
            total += test_phi_recursive(i, 3);
        }
        
        /* This one should be especially hot */
        total += test_hot_path_annotation(i % 50);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
