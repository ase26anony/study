/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int) __attribute__((noinline));
void use(int x) {
    /* Empty implementation - linker will resolve */
}

/* Global checksum to accumulate results */
static volatile int checksum = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
void test_phi_copy_compare_zero(volatile int cond) {
    int x;
    
    /* Create phi node with constants 0 and 1 */
    if (cond & 1) {
        x = 0;  /* First phi argument */
    } else {
        x = 1;  /* Second phi argument */
    }
    
    /* Create copy propagation chain */
    int a = x;      /* First copy */
    int b = a;      /* Second copy */
    int c = b;      /* Third copy - final in chain */
    
    /* Conditional branch comparing to constant 0 */
    if (c == 0) {
        use(100);
        checksum += 1;
    } else {
        use(200);
        checksum += 2;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
void test_phi_long_chain_compare_one(volatile int cond) {
    int y;
    
    if (cond & 2) {
        y = 1;  /* First phi argument */
    } else {
        y = 0;  /* Second phi argument */
    }
    
    /* Longer copy chain */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;  /* Final in chain */
    
    /* Compare to constant 1 */
    if (t5 == 1) {
        use(300);
        checksum += 4;
    } else {
        use(400);
        checksum += 8;
    }
}

/* Test 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
void test_phi_three_way(volatile int cond) {
    int z;
    
    /* Three-way phi */
    if (cond % 3 == 0) {
        z = 0;
    } else if (cond % 3 == 1) {
        z = 1;
    } else {
        z = 0;  /* Another 0 to test phi with duplicate values */
    }
    
    /* Copy chain */
    int m = z;
    int n = m;
    
    /* Compare to 0 */
    if (n == 0) {
        use(500);
        checksum += 16;
    } else {
        use(600);
        checksum += 32;
    }
}

/* Test 4: Phi in loop context */
__attribute__((noinline, noipa))
void test_phi_in_loop(volatile int iterations) {
    int total = 0;
    
    for (int i = 0; i < (iterations & 3) + 1; i++) {
        int val;
        
        /* Phi inside loop */
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional branch */
        if (tmp2 == 1) {
            total += 10;
        } else {
            total += 20;
        }
    }
    
    checksum += total;
    use(total);
}

/* Test 5: Nested phi-copy patterns */
__attribute__((noinline, noipa))
void test_nested_phi_patterns(volatile int cond1, volatile int cond2) {
    int result;
    
    /* Outer phi */
    if (cond1 & 1) {
        int inner;
        
        /* Inner phi */
        if (cond2 & 1) {
            inner = 1;
        } else {
            inner = 0;
        }
        
        /* Copy chain */
        int c1 = inner;
        int c2 = c1;
        
        /* Conditional on inner copy */
        if (c2 == 0) {
            result = 1000;
        } else {
            result = 2000;
        }
    } else {
        result = 3000;
    }
    
    /* Final copy and conditional */
    int final = result;
    if (final > 1500) {
        checksum += 64;
        use(700);
    } else {
        checksum += 128;
        use(800);
    }
}

/* Test 6: Phi with boolean constants from different types */
__attribute__((noinline, noipa))
void test_phi_bool_constants(volatile int cond) {
    _Bool flag;
    
    /* Phi with _Bool type */
    if (cond & 4) {
        flag = 0;  /* false */
    } else {
        flag = 1;  /* true */
    }
    
    /* Copy through int */
    int as_int = flag;
    int another = as_int;
    
    /* Compare to 1 (true) */
    if (another == 1) {
        use(900);
        checksum += 256;
    } else {
        use(1000);
        checksum += 512;
    }
}

/* Test 7: Multiple phi nodes feeding into each other */
__attribute__((noinline, noipa))
void test_phi_chain(volatile int cond) {
    int stage1;
    
    /* First phi */
    if (cond & 1) {
        stage1 = 0;
    } else {
        stage1 = 1;
    }
    
    int copy1 = stage1;
    
    int stage2;
    /* Second phi using copy1 in condition */
    if (copy1 == 0) {
        stage2 = 1;
    } else {
        stage2 = 0;
    }
    
    int copy2 = stage2;
    int copy3 = copy2;
    
    /* Final conditional */
    if (copy3 == 0) {
        use(1100);
        checksum += 1024;
    } else {
        use(1200);
        checksum += 2048;
    }
}

/* Main function that drives all tests */
int main(void) {
    volatile int seed = 0;
    
    /* Initialize with some value */
    seed = 42;
    
    /* Run all test patterns */
    test_phi_copy_compare_zero(seed);
    test_phi_long_chain_compare_one(seed);
    test_phi_three_way(seed);
    test_phi_in_loop(seed);
    test_nested_phi_patterns(seed, seed + 1);
    test_phi_bool_constants(seed);
    test_phi_chain(seed);
    
    /* Print checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
