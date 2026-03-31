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
int test_phi_basic_compare_zero(volatile int cond) {
    int val;
    if (cond > 0) {
        val = 1;  /* Branch 1: constant 1 */
    } else {
        val = 0;  /* Branch 2: constant 0 */
    }
    /* Creates phi node: val = PHI<1, 0> */
    
    int copy1 = val;  /* GIMPLE_ASSIGN with SSA_NAME rhs */
    
    /* Conditional comparing copy to 0 */
    if (copy1 == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy propagation chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_chain_compare_one(volatile int cond) {
    int val;
    if (cond & 1) {
        val = 0;  /* constant 0 */
    } else {
        val = 1;  /* constant 1 */
    }
    /* val = PHI<0, 1> */
    
    int a = val;    /* First copy */
    int b = a;      /* Second copy */
    int c = b;      /* Third copy */
    int d = c;      /* Fourth copy */
    
    /* Compare final copy to 1 */
    if (d == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi with three incoming edges (if-else if-else) */
__attribute__((noinline, noipa))
int test_phi_three_way(volatile int cond) {
    int val;
    if (cond > 10) {
        val = 1;
    } else if (cond < -10) {
        val = 0;
    } else {
        val = 1;  /* Same constant as first branch */
    }
    /* val = PHI<1, 0, 1> */
    
    int tmp = val;
    int tmp2 = tmp;
    
    if (tmp2 != 0) {  /* Compare to 0 with != operator */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Phi inside loop with copy chain */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int limit = iterations & 3;  /* 0-3 iterations */
    
    for (int i = 0; i < limit; i++) {
        int val;
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        /* val = PHI<1, 0> inside loop */
        
        int x = val;
        int y = x;
        
        if (y == 0) {
            sum += 10;
        } else {
            sum += 20;
        }
    }
    return sum;
}

/* Test 5: Nested phi-copy-conditional pattern */
__attribute__((noinline, noipa))
int test_nested_phi(volatile int cond1, volatile int cond2) {
    int val1, val2;
    
    /* First phi */
    if (cond1 > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    /* Second phi dependent on first */
    if (val1 == 0) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    /* val2 = PHI<1, 0> */
    
    int chain1 = val2;
    int chain2 = chain1;
    int chain3 = chain2;
    
    if (chain3 == 1) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 6: Phi with boolean constants from function arguments */
__attribute__((noinline, noipa))
int test_phi_from_args(int flag1, int flag2) {
    int val;
    if (flag1) {
        val = 1;
    } else if (flag2) {
        val = 0;
    } else {
        val = 1;
    }
    
    int copy = val;
    if (copy == 0) {
        return 900;
    }
    return 901;
}

/* Dummy use function to prevent optimization */
void use(int x) {
    /* Empty in this test - linker will resolve */
}

/* Main function that exercises all patterns */
int main() {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Run all tests with varying conditions */
    checksum += test_phi_basic_compare_zero(g_seed);
    checksum += test_phi_chain_compare_one(g_seed + 1);
    checksum += test_phi_three_way(g_seed - 50);
    checksum += test_phi_in_loop(g_seed);
    checksum += test_nested_phi(g_seed, g_seed / 2);
    checksum += test_phi_from_args(1, 0);
    checksum += test_phi_from_args(0, 1);
    checksum += test_phi_from_args(0, 0);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs with different seeds */
    for (int i = 0; i < 5; i++) {
        g_seed = i * 17;
        test_phi_basic_compare_zero(g_seed);
        test_phi_chain_compare_one(g_seed);
    }
    
    return 0;
}
