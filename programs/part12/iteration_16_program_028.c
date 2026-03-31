/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime decisions */
volatile int g_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test1_basic_phi_compare_zero(volatile int cond) {
    int val;
    if (cond > 0) {
        val = 1;  /* Becomes phi argument 1 */
    } else {
        val = 0;  /* Becomes phi argument 0 */
    }
    
    /* Single copy assignment */
    int copy1 = val;
    
    /* Conditional branch comparing to 0 */
    if (copy1 == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy chain (3 copies), compare to 1 */
__attribute__((noinline, noipa))
int test2_long_chain_compare_one(volatile int cond) {
    int val;
    if (cond & 1) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Chain of copy assignments */
    int a = val;
    int b = a;
    int c = b;
    
    /* Compare to 1 */
    if (c == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi with multiple incoming edges (3-way branch) */
__attribute__((noinline, noipa))
int test3_multiway_phi(volatile int cond) {
    int val;
    if (cond % 3 == 0) {
        val = 0;
    } else if (cond % 3 == 1) {
        val = 1;
    } else {
        val = 0;  /* Another 0 to test phi with duplicate values */
    }
    
    /* Two-level copy chain */
    int x = val;
    int y = x;
    
    /* Compare to 0 with != operator */
    if (y != 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Nested loops with phi-copy-conditional pattern */
__attribute__((noinline, noipa))
int test4_loop_context(volatile int iter_count) {
    int sum = 0;
    volatile int local_seed = iter_count;
    
    for (int i = 0; i < (local_seed % 5) + 1; i++) {
        int val;
        if (local_seed & (1 << i)) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional that affects loop state */
        if (tmp2 == 0) {
            sum += i * 10;
        } else {
            sum += i * 20;
        }
    }
    return sum;
}

/* Test 5: Phi with boolean constants from function arguments */
__attribute__((noinline, noipa))
int test5_arg_based_phi(int flag1, int flag2) {
    int val;
    if (flag1) {
        val = 1;
    } else if (flag2) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Longer copy chain */
    int v1 = val;
    int v2 = v1;
    int v3 = v2;
    int v4 = v3;
    
    /* Compare to 1 */
    if (v4 == 1) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 6: Switch statement creating phi nodes */
__attribute__((noinline, noipa))
int test6_switch_phi(volatile int code) {
    int val;
    switch (code % 4) {
        case 0: val = 0; break;
        case 1: val = 1; break;
        case 2: val = 0; break;
        case 3: val = 1; break;
        default: val = 0; break;
    }
    
    /* Minimal copy */
    int final = val;
    
    /* Conditional with both 0 and 1 comparisons */
    if (final == 0) {
        use(900);
        return 9;
    } else if (final == 1) {
        use(1000);
        return 10;
    }
    return 0;
}

/* Test 7: Complex control flow with multiple phis merging */
__attribute__((noinline, noipa))
int test7_complex_merge(volatile int a, volatile int b) {
    int val1, val2;
    
    if (a > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    if (b > 0) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    
    /* Phi for the result of logical AND */
    int result;
    if (val1 && val2) {
        result = 1;
    } else {
        result = 0;
    }
    
    /* Copy chain */
    int r1 = result;
    int r2 = r1;
    
    /* Final conditional */
    if (r2 == 1) {
        use(1100);
        return 11;
    } else {
        use(1200);
        return 12;
    }
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

int main() {
    int checksum = 0;
    
    /* Initialize volatile seed */
    volatile int seed = g_seed;
    if (seed == 0) {
        seed = 12345;  /* Default seed if not set externally */
    }
    
    /* Run all test patterns */
    checksum += test1_basic_phi_compare_zero(seed);
    checksum += test2_long_chain_compare_one(seed + 1);
    checksum += test3_multiway_phi(seed + 2);
    checksum += test4_loop_context(seed + 3);
    checksum += test5_arg_based_phi((seed >> 1) & 1, (seed >> 2) & 1);
    checksum += test6_switch_phi(seed + 4);
    checksum += test7_complex_merge(seed + 5, seed + 6);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs with different seeds to exercise more paths */
    for (int i = 0; i < 3; i++) {
        test1_basic_phi_compare_zero(seed + i * 10);
        test2_long_chain_compare_one(seed + i * 20);
    }
    
    return 0;
}
