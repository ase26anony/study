/* test_auto_profile.c - Test program for GCC auto-profile coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization */
static volatile int vol_counter = 0;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    int b = a;
    int c = b;
    vol_counter++; /* Prevent optimization */
    return c;
}

/* Function with phi node from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0; /* This will become a phi node */
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* Hot path */
        } else {
            flag = 0;  /* Cold path */
        }
        result += i;
    }
    
    /* Create copy chain */
    int copied_flag = copy_chain(flag);
    
    /* Critical conditional comparing against 0/1 */
    if (copied_flag == 1) {  /* Compare against 1 */
        return result * 2;    /* Hot path */
    } else {
        return result / 2;    /* Cold path */
    }
}

/* Function with phi from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple returns create phi for return value */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Equal case */
    }
    
    /* Multiple assignments to create copy chain */
    int a = condition;
    int b = a;
    int c = b;
    int d = c;
    
    /* Conditional with comparison against 0 */
    if (d == 0) {  /* Compare against 0 */
        return x - y;
    } else {
        return x + y;
    }
}

/* Function with phi from switch statement */
int test_phi_from_switch(int mode) {
    int status = 0;  /* Will become phi node */
    
    switch (mode % 4) {
        case 0:
            status = 1;  /* Hot */
            break;
        case 1:
            status = 0;  /* Cold */
            break;
        case 2:
            status = 1;  /* Hot */
            break;
        case 3:
            status = 0;  /* Cold */
            break;
    }
    
    /* Extended copy chain */
    int s1 = status;
    int s2 = s1;
    int s3 = s2;
    
    /* Boolean context (implicit comparison with 0) */
    if (s3) {  /* Equivalent to s3 != 0 */
        return mode * 10;
    } else {
        return mode * 5;
    }
}

/* Function with recursive phi creation */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int next_val;
    if (n % 2 == 0) {
        next_val = test_recursive_phi(n / 2, depth - 1);
    } else {
        next_val = test_recursive_phi(n * 3 + 1, depth - 1);
    }
    
    /* Phi created from recursive calls */
    int phi_val = (next_val > 100) ? 1 : 0;
    
    /* Copy chain */
    int v1 = phi_val;
    int v2 = v1;
    
    /* Compare against 1 */
    if (v2 == 1) {
        return next_val + 100;
    } else {
        return next_val - 100;
    }
}

/* Function with global variable creating phi */
static int global_counter = 0;
int test_phi_with_global(int limit) {
    int local_flag;
    
    /* Global creates phi when used in loop */
    for (int i = 0; i < limit; ++i) {
        if (global_counter % 2 == 0) {
            local_flag = 1;
        } else {
            local_flag = 0;
        }
        global_counter++;
    }
    
    /* Multiple copy assignments */
    int f1 = local_flag;
    int f2 = f1;
    int f3 = f2;
    
    /* Compare against 0 using inequality */
    if (f3 != 0) {  /* Tests f3 == 1 indirectly */
        return global_counter * 2;
    } else {
        return global_counter / 2;
    }
}

/* Function with ternary operator creating 0/1 value */
int test_phi_from_ternary(int a, int b) {
    /* Ternary produces 0/1 */
    int is_greater = (a > b) ? 1 : 0;
    
    /* Copy chain through volatile */
    volatile int temp = is_greater;
    int chain1 = temp;
    int chain2 = chain1;
    
    /* Direct comparison with 1 */
    if (chain2 == 1) {
        return a;
    } else {
        return b;
    }
}

/* Main function to create hot execution paths */
int main(void) {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Hot loop to trigger profile annotation */
    for (int i = 0; i < ITERATIONS; ++i) {
        /* Call all test functions to create various phi patterns */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        total += test_recursive_phi(i, 3);
        total += test_phi_with_global(i % 50 + 1);
        total += test_phi_from_ternary(i, ITERATIONS - i);
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect((i % 10) == 0, 1)) {
            total += i;  /* Hot path */
        }
    }
    
    printf("Result: %d (volatile counter: %d)\n", total, vol_counter);
    return total > 0 ? 0 : 1;
}
