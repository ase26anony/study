/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_zero(volatile int cond) {
    int result;
    
    /* Create phi node with constants 0 and 1 */
    if (cond > 0) {
        result = 1;  /* Branch 1: constant 1 */
    } else {
        result = 0;  /* Branch 2: constant 0 */
    }
    
    /* Create copy propagation chain */
    int a = result;  /* First copy */
    int b = a;       /* Second copy */
    int c = b;       /* Third copy */
    int d = c;       /* Fourth copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (d == 0) {
        use(100);    /* Taken when d == 0 */
        return 1;
    } else {
        use(200);    /* Taken when d == 1 */
        return 2;
    }
}

/* Test function 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(volatile int cond) {
    int val;
    
    /* Different branch structure for phi */
    if (cond & 1) {
        val = 1;
    } else if (cond & 2) {
        val = 0;
    } else {
        val = 1;  /* Third incoming edge to phi */
    }
    
    /* Longer copy chain */
    int t1 = val;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    int t6 = t5;
    
    /* Compare to 1 (not 0) */
    if (t6 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < (loop_cond & 3) + 1; i++) {
        int temp;
        
        /* Phi node inside loop */
        if (g_seed + i > 0) {
            temp = 1;
        } else {
            temp = 0;
        }
        
        /* Copy chain */
        int x = temp;
        int y = x;
        
        /* Conditional based on copy */
        if (y == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
    }
    
    use(sum);
    return sum;
}

/* Test function 4: Nested phi-copy pattern */
__attribute__((noinline, noipa))
int test_nested_phi_pattern(volatile int a, volatile int b) {
    int phi_result1, phi_result2;
    
    /* First phi node */
    if (a > 0) {
        phi_result1 = 1;
    } else {
        phi_result1 = 0;
    }
    
    /* Second phi node */
    if (b > 0) {
        phi_result2 = 1;
    } else {
        phi_result2 = 0;
    }
    
    /* Copy chains for both */
    int copy1a = phi_result1;
    int copy1b = copy1a;
    
    int copy2a = phi_result2;
    int copy2b = copy2a;
    
    /* Multiple conditionals */
    int result = 0;
    if (copy1b == 0) {
        result += 10;
    }
    
    if (copy2b == 1) {
        result += 20;
    }
    
    use(result);
    return result;
}

/* Test function 5: Phi with boolean operations in copy chain */
__attribute__((noinline, noipa))
int test_phi_with_boolean_ops(volatile int cond1, volatile int cond2) {
    int flag;
    
    /* Phi with 0/1 constants */
    if (cond1 > cond2) {
        flag = 1;
    } else {
        flag = 0;
    }
    
    /* Simple copy chain */
    int f1 = flag;
    int f2 = f1;
    
    /* Compare to 0 using inequality */
    if (f2 != 0) {  /* Should become f2 == 1 in GIMPLE */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 6: Switch-based phi pattern */
__attribute__((noinline, noipa))
int test_switch_phi_pattern(volatile int selector) {
    int value;
    
    /* Switch creates multiple incoming edges to phi */
    switch (selector & 3) {
        case 0: value = 0; break;
        case 1: value = 1; break;
        case 2: value = 0; break;
        default: value = 1; break;
    }
    
    /* Copy chain */
    int v1 = value;
    int v2 = v1;
    int v3 = v2;
    
    /* Conditional */
    if (v3 == 1) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Dummy use function to prevent dead code elimination */
void use(int x) {
    /* Empty - will be optimized away, but keeps conditional alive */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    static volatile void* sink;
    sink = p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Call test functions with different inputs */
    checksum += test_phi_copy_compare_zero(g_seed);
    checksum += test_phi_copy_compare_zero(-g_seed);
    
    checksum += test_phi_long_chain_compare_one(g_seed);
    checksum += test_phi_long_chain_compare_one(g_seed + 1);
    
    checksum += test_phi_in_loop(g_seed);
    checksum += test_phi_in_loop(g_seed + 2);
    
    checksum += test_nested_phi_pattern(g_seed, -g_seed);
    checksum += test_nested_phi_pattern(-g_seed, g_seed);
    
    checksum += test_phi_with_boolean_ops(g_seed, g_seed / 2);
    checksum += test_phi_with_boolean_ops(g_seed / 2, g_seed);
    
    checksum += test_switch_phi_pattern(g_seed);
    checksum += test_switch_phi_pattern(g_seed + 1);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
