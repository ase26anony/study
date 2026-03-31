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

/* Test function 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
void test_phi_basic_compare_zero(volatile int cond) {
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
        checksum += 1;
        use(1);
    } else {
        checksum += 2;
        use(2);
    }
}

/* Test function 2: Longer copy chain (3 copies), compare to 1 */
__attribute__((noinline, noipa))
void test_phi_long_chain_compare_one(volatile int cond) {
    int val;
    
    /* Different branch structure to create phi */
    if (cond & 1) {
        val = 0;
    } else if (cond & 2) {
        val = 1;
    } else {
        val = 0;  /* Third phi argument */
    }
    
    /* Chain of 3 copy assignments */
    int a = val;
    int b = a;
    int c = b;
    
    /* Compare final copy to 1 */
    if (c == 1) {
        checksum += 3;
        use(3);
    } else {
        checksum += 4;
        use(4);
    }
}

/* Test function 3: Phi with copy chain in loop, compare to 0 */
__attribute__((noinline, noipa))
void test_phi_loop_compare_zero(volatile int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        
        /* Phi node created at loop header */
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional based on copy */
        if (tmp2 == 0) {
            total += i;
            use(i);
        } else {
            total -= i;
            use(-i);
        }
    }
    
    checksum += total;
}

/* Test function 4: Nested phi nodes with copy propagation */
__attribute__((noinline, noipa))
void test_nested_phi_copies(volatile int cond1, volatile int cond2) {
    int val1, val2;
    
    /* First level phi */
    if (cond1 > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    /* Copy then another conditional */
    int copy1 = val1;
    
    if (cond2 > 0) {
        val2 = copy1;  /* Phi with SSA_NAME as argument */
    } else {
        val2 = 1;
    }
    
    /* Final copy and compare */
    int final = val2;
    
    if (final == 0) {
        checksum += 10;
        use(10);
    } else {
        checksum += 20;
        use(20);
    }
}

/* Test function 5: Phi with boolean constants from different types */
__attribute__((noinline, noipa))
void test_phi_bool_constants(volatile int cond) {
    _Bool bval;
    
    if (cond % 3 == 0) {
        bval = 1;  /* Boolean true */
    } else if (cond % 3 == 1) {
        bval = 0;  /* Boolean false */
    } else {
        bval = 1;  /* Boolean true */
    }
    
    /* Convert to int through copy chain */
    int ival = (int)bval;
    int copy1 = ival;
    int copy2 = copy1;
    
    /* Compare to 1 (boolean true) */
    if (copy2 == 1) {
        checksum += 100;
        use(100);
    } else {
        checksum += 200;
        use(200);
    }
}

/* Test function 6: Switch-based phi with copy chain */
__attribute__((noinline, noipa))
void test_switch_phi_copies(volatile int selector) {
    int val;
    
    switch (selector % 4) {
        case 0: val = 0; break;
        case 1: val = 1; break;
        case 2: val = 0; break;
        default: val = 1; break;
    }
    
    /* Longer copy chain (4 copies) */
    int a = val;
    int b = a;
    int c = b;
    int d = c;
    
    /* Compare to 0 */
    if (d == 0) {
        checksum += 50;
        use(50);
    } else {
        checksum += 60;
        use(60);
    }
}

/* Test function 7: Phi with arithmetic between copies */
__attribute__((noinline, noipa))
void test_phi_arithmetic_copies(volatile int cond) {
    int val;
    
    if (cond > 100) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Copy with simple arithmetic that doesn't break SSA chain */
    int a = val;
    int b = a + 0;  /* Still a single assignment with SSA_NAME */
    int c = b;
    
    /* Compare to 1 */
    if (c != 0) {  /* Equivalent to c == 1 for boolean values */
        checksum += 70;
        use(70);
    } else {
        checksum += 80;
        use(80);
    }
}

/* Main function to drive all tests */
int main(void) {
    volatile int seed = 0;
    
    /* Initialize with volatile read to prevent constant propagation */
    seed = rand() % 1000;
    
    /* Run all test functions with varying conditions */
    test_phi_basic_compare_zero(seed);
    test_phi_long_chain_compare_one(seed + 1);
    test_phi_loop_compare_zero(seed % 5 + 1);
    test_nested_phi_copies(seed, seed + 100);
    test_phi_bool_constants(seed);
    test_switch_phi_copies(seed);
    test_phi_arithmetic_copies(seed * 2);
    
    /* Print checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
