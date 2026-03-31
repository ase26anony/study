/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Simple phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_simple_copy_compare_zero(int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Will become phi argument 1 */
    } else {
        x = 0;  /* Will become phi argument 0 */
    }
    
    /* Copy chain: phi -> a -> b (condition) */
    int a = x;      /* First copy */
    int b = a;      /* Second copy */
    
    /* Conditional branch comparing copy to 0 */
    if (b == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(int cond) {
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
    
    /* Compare to 1 (not 0) */
    if (t4 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with three incoming edges (if-else if-else) */
__attribute__((noinline, noipa))
int test_phi_three_way(int cond) {
    int z;
    if (cond > 10) {
        z = 1;
    } else if (cond < -10) {
        z = 0;
    } else {
        z = 1;  /* Same as first branch to test phi merging */
    }
    
    /* Copy through multiple variables */
    int copy1 = z;
    int copy2 = copy1;
    
    /* Compare copy to 0 */
    if (copy2 == 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    volatile int vol = g_seed;  /* Prevent loop unrolling */
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        /* Create phi at loop header */
        if (vol & (1 << i)) {
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
    }
    return sum;
}

/* Test function 5: Nested phi-copy pattern */
__attribute__((noinline, noipa))
int test_nested_phi_copy(int cond1, int cond2) {
    int a, b;
    
    /* First phi */
    if (cond1 > 0) {
        a = 1;
    } else {
        a = 0;
    }
    
    /* Second phi */
    if (cond2 > 0) {
        b = 1;
    } else {
        b = 0;
    }
    
    /* Copy both */
    int a_copy = a;
    int b_copy = b;
    
    /* Two conditionals to test */
    int result = 0;
    if (a_copy == 0) {
        result += 10;
    }
    if (b_copy == 1) {
        result += 20;
    }
    
    use(result);
    return result;
}

/* Test function 6: Phi with != comparison instead of == */
__attribute__((noinline, noipa))
int test_phi_not_equal(int cond) {
    int x;
    if (cond % 3 == 0) {
        x = 1;
    } else {
        x = 0;
    }
    
    int y = x;
    int z = y;
    
    /* Use != comparison (should still create GIMPLE_COND) */
    if (z != 0) {  /* Equivalent to z == 1 */
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test function 7: Multiple uses of same phi through different copy chains */
__attribute__((noinline, noipa))
int test_phi_multiple_chains(int cond) {
    int base;
    if (cond > 5) {
        base = 1;
    } else {
        base = 0;
    }
    
    /* Two independent copy chains from same phi */
    int chain1_a = base;
    int chain1_b = chain1_a;
    
    int chain2_a = base;
    int chain2_b = chain2_a;
    int chain2_c = chain2_b;
    
    /* Two conditionals using different chains */
    int result = 0;
    if (chain1_b == 0) {
        result += 1000;
    }
    if (chain2_c == 1) {
        result += 2000;
    }
    
    return result;
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    static volatile void* sink;
    sink = p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = g_seed;
    
    /* Call all test functions with different conditions */
    checksum += test_phi_simple_copy_compare_zero(seed);
    checksum += test_phi_long_chain_compare_one(seed + 1);
    checksum += test_phi_three_way(seed);
    checksum += test_phi_in_loop(5);
    checksum += test_nested_phi_copy(seed, -seed);
    checksum += test_phi_not_equal(seed);
    checksum += test_phi_multiple_chains(seed + 3);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs with different seeds to exercise different paths */
    for (int i = 0; i < 3; i++) {
        g_seed = i;
        test_phi_simple_copy_compare_zero(i * 10);
        test_phi_long_chain_compare_one(i * 7);
    }
    
    return 0;
}
