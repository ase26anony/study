/* Test program for GCC auto-profile coverage of phi-node-defined conditionals */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Function with loop-carried phi node */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
    }
    
    /* 'flag' is defined by phi node here */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
    /* Critical conditional comparing against 0/1 */
    if (flag_copy == 0) {  /* Compare against 0 */
        result += 1;
    } else if (flag_copy == 1) {  /* Compare against 1 */
        result += 2;
    }
    
    return result;
}

/* Function with switch creating phi node */
int test_phi_from_switch(int mode) {
    int status = 0;
    
    switch (mode % 4) {
        case 0:
            status = 1;  /* First incoming edge */
            break;
        case 1:
            status = 0;  /* Second incoming edge */
            break;
        case 2:
            status = 1;  /* Third incoming edge */
            break;
        default:
            status = 0;  /* Fourth incoming edge */
    }
    
    /* 'status' defined by phi node from switch */
    int s1 = status;
    int s2 = s1;  /* Copy chain */
    int s3 = s2;  /* Another copy */
    
    /* Conditional with 0/1 comparison */
    if (s3 == 1) {  /* Hot path - compare against 1 */
        return 100;
    } else if (s3 == 0) {  /* Compare against 0 */
        return 200;
    }
    
    return 0;
}

/* Function with multiple returns creating phi */
bool test_phi_from_multiple_returns(int x) {
    if (x > 1000) {
        return true;  /* First incoming value */
    }
    if (x < 0) {
        return false; /* Second incoming value */
    }
    return (x % 2) == 0; /* Third incoming value */
}

void test_phi_with_bool(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Call creates phi node for return value (bool) */
        bool cond = test_phi_from_multiple_returns(i);
        
        /* Create copy chain for the bool */
        bool c1 = cond;
        bool c2 = c1;
        bool c3 = c2;
        
        /* Boolean comparison (implicit 0/1) */
        if (c3) {  /* if (c3 != 0) */
            sum += i;
        } else if (!c3) {  /* if (c3 == 0) */
            sum -= i;
        }
    }
    
    volatile_sink = sum;  /* Use result */
}

/* Recursive function creating phi for depth */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0 || n <= 0) {
        return 0;  /* First incoming value */
    }
    
    int recurse = test_recursive_phi(n - 1, depth - 1);
    
    /* 'recurse' gets phi node from recursive calls */
    int r1 = recurse;
    int r2 = r1;
    
    /* Compare against 0 */
    if (r2 == 0) {
        return n + depth;
    } else if (r2 == 1) {
        return n * depth;
    }
    
    return recurse;
}

/* Function with conditional inside hot loop */
int test_hot_path_conditional(int iterations) {
    int total = 0;
    int state = 0;
    
    /* Hot loop - will be annotated as hot */
    for (int i = 0; i < iterations; i++) {
        /* Update state - creates phi node */
        if (i % 10 == 0) {
            state = 1;
        } else if (i % 3 == 0) {
            state = 0;
        } else {
            state = (state + 1) % 2;
        }
        
        /* Copy chain */
        int s = state;
        int t = s;
        int u = t;
        
        /* Hot conditional comparing against 0/1 */
        if (__builtin_expect(u == 1, 1)) {  /* Hint as hot path */
            total += i * 2;
        } else if (u == 0) {
            total += i;
        }
    }
    
    return total;
}

/* Main driver with hot execution */
int main(void) {
    int result = 0;
    
    /* Execute many times to create hot paths */
    for (int outer = 0; outer < 1000; outer++) {
        /* Test 1: Loop-carried phi */
        result += test_phi_after_loop(100);
        
        /* Test 2: Switch-based phi */
        result += test_phi_from_switch(outer);
        
        /* Test 3: Hot path with phi */
        result += test_hot_path_conditional(1000);
        
        /* Test 4: Recursive phi */
        result += test_recursive_phi(10, 5);
        
        /* Test 5: Boolean phi from multiple returns */
        test_phi_with_bool(500);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
