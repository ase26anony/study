/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime evaluation */
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
    
    /* Conditional branch comparing copy to 0 */
    if (copy1 == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test2_phi_long_chain_compare_one(volatile int cond) {
    int val;
    if (cond & 1) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Create a chain of 3 copy assignments */
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

/* Test 3: Phi with multiple incoming edges (3-way) */
__attribute__((noinline, noipa))
int test3_phi_three_way(volatile int cond) {
    int val;
    if (cond < 0) {
        val = 0;
    } else if (cond == 0) {
        val = 1;
    } else {
        val = 0;  /* Different phi argument but same value as first */
    }
    
    int tmp = val;
    int tmp2 = tmp;
    
    if (tmp2 != 1) {  /* Compare to 1 with != operator */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Phi inside loop with copy propagation */
__attribute__((noinline, noipa))
int test4_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < loop_cond && i < 10; i++) {
        int val;
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int a = val;
        int b = a;
        
        if (b == 0) {
            sum += i;
            use(sum);
        } else {
            sum -= i;
            use(sum);
        }
    }
    return sum;
}

/* Test 5: Nested phi nodes with copy chains */
__attribute__((noinline, noipa))
int test5_nested_phi(volatile int cond1, volatile int cond2) {
    int val1, val2;
    
    if (cond1 > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    if (cond2 > 0) {
        val2 = val1;  /* Uses val1 which is phi result */
    } else {
        val2 = 0;
    }
    
    /* val2 is phi with one argument being another phi result */
    int copy = val2;
    
    if (copy == 0) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 6: Phi with boolean constants from different types of branches */
__attribute__((noinline, noipa))
int test6_mixed_branches(volatile int cond) {
    int val;
    
    switch (cond % 3) {
        case 0:
            val = 1;
            break;
        case 1:
            val = 0;
            break;
        default:
            val = 1;
            break;
    }
    
    int x = val;
    int y = x;
    int z = y;
    
    if (z != 0) {  /* != comparison */
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test 7: Multiple phi users with different copy chains */
__attribute__((noinline, noipa))
int test7_multiple_phi_users(volatile int cond) {
    int val;
    if (cond > 100) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Two independent copy chains from same phi */
    int chain1_a = val;
    int chain1_b = chain1_a;
    
    int chain2_a = val;
    int chain2_b = chain2_a;
    int chain2_c = chain2_b;
    
    /* Two separate conditionals using different chains */
    int result = 0;
    if (chain1_b == 0) {
        result += 11;
        use(result);
    }
    
    if (chain2_c == 1) {
        result += 22;
        use(result);
    }
    
    return result;
}

/* Main function that exercises all patterns */
int main() {
    volatile int seed = g_seed;
    int checksum = 0;
    
    /* Call each test with different volatile conditions */
    checksum += test1_basic_phi_compare_zero(seed);
    checksum += test2_phi_long_chain_compare_one(seed + 1);
    checksum += test3_phi_three_way(seed - 1);
    checksum += test4_phi_in_loop(seed % 5 + 3);
    checksum += test5_nested_phi(seed, seed + 1);
    checksum += test6_mixed_branches(seed * 2);
    checksum += test7_multiple_phi_users(seed + 50);
    
    printf("Checksum: %d\n", checksum);
    
    /* Force multiple runs with different seeds for profile generation */
    for (int i = 0; i < 100; i++) {
        g_seed = i;
        test1_basic_phi_compare_zero(g_seed);
        test2_phi_long_chain_compare_one(g_seed);
    }
    
    return 0;
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(int* p) {
    static volatile int* sink_ptr;
    sink_ptr = p;
}
