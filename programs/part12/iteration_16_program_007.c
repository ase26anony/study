/* test_autofdo_phi_copy_cond.c
 * Generates GIMPLE patterns with phi nodes, copy chains, and conditional branches
 * to exercise uncovered AutoFDO profile reading logic.
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_zero(volatile int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Becomes phi argument 1 */
    } else {
        x = 0;  /* Becomes phi argument 0 */
    }
    
    /* Copy chain: phi -> a -> b (condition) */
    int a = x;      /* GIMPLE_ASSIGN copying SSA_NAME */
    int b = a;      /* Another copy */
    
    /* Conditional comparing copy to 0 */
    if (b == 0) {   /* GIMPLE_COND with cmp_rhs = 0, cmp_lhs = b (SSA_NAME) */
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_long_chain_compare_one(volatile int cond) {
    int y;
    if (cond & 1) {
        y = 0;
    } else {
        y = 1;
    }
    
    /* Longer copy chain: phi -> t1 -> t2 -> t3 -> t4 (condition) */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    
    /* Compare to 1 */
    if (t4 == 1) {  /* cmp_rhs = 1 */
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi with three incoming edges (basic block with 3 predecessors) */
__attribute__((noinline, noipa))
int test_phi_three_way(volatile int mode) {
    int z;
    if (mode == 0) {
        z = 1;
    } else if (mode == 1) {
        z = 0;
    } else {
        z = 1;  /* Third phi argument */
    }
    
    /* Copy through multiple variables */
    int copy1 = z;
    int copy2 = copy1;
    
    /* Compare to 0 with != instead of == */
    if (copy2 != 0) {  /* Still GIMPLE_COND with constant 0 */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        /* Phi node inside loop */
        if (loop_cond & (1 << i)) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional based on copy */
        if (tmp2 == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
        
        /* Modify loop condition to vary phi inputs */
        loop_cond ^= (i + 1);
    }
    return sum;
}

/* Test 5: Nested conditionals creating complex phi patterns */
__attribute__((noinline, noipa))
int test_nested_phi_copy(volatile int a, volatile int b) {
    int result;
    
    if (a > 0) {
        if (b > 0) {
            result = 1;
        } else {
            result = 0;
        }
    } else {
        if (b < 0) {
            result = 1;
        } else {
            result = 0;
        }
    }
    
    /* Multi-step copy chain */
    int stage1 = result;
    int stage2 = stage1;
    int stage3 = stage2;
    int stage4 = stage3;
    int final = stage4;
    
    /* Final conditional */
    if (final == 1) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 6: Phi with boolean constants from different types of comparisons */
__attribute__((noinline, noipa))
int test_mixed_comparisons(volatile int x, volatile int y) {
    int flag;
    
    /* Different comparisons feeding phi */
    if (x > y) {
        flag = 1;
    } else if (x == y) {
        flag = 0;
    } else {
        flag = 1;  /* x < y */
    }
    
    /* Minimal copy chain */
    int f1 = flag;
    
    /* Compare to 1 */
    if (f1 == 1) {
        use(900);
        return 9;
    }
    use(1000);
    return 10;
}

/* Main function that exercises all patterns */
int main() {
    volatile int seed = g_seed;
    int checksum = 0;
    
    /* Call each test with varying inputs to exercise different paths */
    checksum += test_phi_copy_compare_zero(seed);
    checksum += test_long_chain_compare_one(seed + 1);
    checksum += test_phi_three_way(seed % 3);
    checksum += test_phi_in_loop(5 + (seed % 5));
    checksum += test_nested_phi_copy(seed, ~seed);
    checksum += test_mixed_comparisons(seed, seed / 2);
    
    printf("Checksum: %d\n", checksum);
    
    /* Also run with different seeds if compiled with profile generation */
    for (int i = 0; i < 10; i++) {
        test_phi_copy_compare_zero(i);
        test_long_chain_compare_one(i * 2);
    }
    
    return 0;
}

/* Dummy definitions to satisfy linker (not needed for compilation) */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(int *p) {
    static volatile int sink;
    if (p) sink = *p;
}
