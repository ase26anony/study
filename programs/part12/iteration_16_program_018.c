/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Basic phi with 0/1 constants, single copy chain */
__attribute__((noinline, noipa))
int test_phi_basic(volatile int cond) {
    int val;
    if (cond > 0) {
        val = 1;  /* Becomes phi argument 1 */
    } else {
        val = 0;  /* Becomes phi argument 0 */
    }
    
    /* Copy propagation chain */
    int a = val;
    int b = a;
    int c = b;
    
    /* Conditional branch comparing to 0 */
    if (c == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain(volatile int cond) {
    int val;
    if (cond & 1) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Longer copy chain */
    int t1 = val;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    /* Compare to 1 */
    if (t5 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
int test_phi_three_way(volatile int cond) {
    int val;
    if (cond > 10) {
        val = 1;
    } else if (cond < -10) {
        val = 0;
    } else {
        val = 1;  /* Same as first branch to test phi merging */
    }
    
    int x = val;
    int y = x;
    
    if (y != 0) {  /* Compare with != instead of == */
        use(500);
        return 5;
    }
    use(600);
    return 6;
}

/* Test function 4: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        if (loop_cond & (1 << i)) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        if (tmp2 == 0) {
            sum += i * 2;
            use(i);
        } else {
            sum += i * 3;
            use(i + 1000);
        }
    }
    return sum;
}

/* Test function 5: Nested phi-copy patterns */
__attribute__((noinline, noipa))
int test_nested_phi(volatile int cond1, volatile int cond2) {
    int val1, val2;
    
    /* First phi */
    if (cond1 > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    /* Second phi */
    if (cond2 > 0) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    
    /* Copy chains */
    int a = val1;
    int b = val2;
    int c = a + b;  /* Mix them */
    
    /* Conditional on mixed value */
    if (c == 0) {
        use(700);
        return 7;
    } else if (c == 1) {
        use(800);
        return 8;
    } else {
        use(900);
        return 9;
    }
}

/* Test function 6: Phi with pointer casts (might create different SSA patterns) */
__attribute__((noinline, noipa))
int test_phi_with_casts(volatile int cond) {
    int val;
    if (cond % 3 == 0) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Copy with type changes */
    int a = val;
    short b = (short)a;
    int c = (int)b;
    char d = (char)c;
    int e = (int)d;
    
    if (e == 0) {
        use_ptr(&e);
        return 10;
    }
    use_ptr(&val);
    return 11;
}

/* Test function 7: Multiple phis feeding into each other */
__attribute__((noinline, noipa))
int test_multi_phi_chain(volatile int cond) {
    int val1, val2;
    
    if (cond > 100) {
        val1 = 1;
        val2 = 0;
    } else {
        val1 = 0;
        val2 = 1;
    }
    
    /* First copy chain */
    int a = val1;
    int b = a;
    
    /* Second copy chain */
    int x = val2;
    int y = x;
    
    /* Choose which chain to use based on condition */
    int final;
    if (cond & 1) {
        final = b;
    } else {
        final = y;
    }
    
    if (final == 1) {
        use(1200);
        return 12;
    }
    use(1300);
    return 13;
}

/* Main function that exercises all patterns */
int main() {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = g_seed;
    if (seed == 0) {
        seed = 1;  /* Ensure non-zero for some branches */
    }
    
    /* Call all test functions with varying inputs */
    checksum += test_phi_basic(seed);
    checksum += test_phi_long_chain(seed + 1);
    checksum += test_phi_three_way(seed - 5);
    checksum += test_phi_in_loop(seed % 8 + 2);
    checksum += test_nested_phi(seed, seed + 1);
    checksum += test_phi_with_casts(seed * 3);
    checksum += test_multi_phi_chain(seed * 2);
    
    printf("Checksum: %d\n", checksum);
    
    /* Also call with different inputs to exercise other paths */
    checksum += test_phi_basic(0);
    checksum += test_phi_long_chain(0);
    checksum += test_phi_three_way(20);
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions to satisfy linker (in real use, these would be external) */
void use(int x) {
    /* Prevent optimization */
    asm volatile("" : : "r"(x) : "memory");
}

void use_ptr(void* p) {
    /* Prevent optimization */
    asm volatile("" : : "r"(p) : "memory");
}
