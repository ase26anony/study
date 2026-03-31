/* Test program for GCC AutoFDO profile reading coverage
 * Specifically targets lines 1312-1333 in auto-profile.cc
 */

#include <stdio.h>
#include <stdint.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global checksum to accumulate results */
static int checksum = 0;

/* Volatile variable to prevent constant propagation */
static volatile int volatile_seed = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
void test_phi_copy_compare_zero(int x) {
    int a;
    
    /* Create phi node with constants 0 and 1 */
    if (x > 0) {
        a = 1;  /* Branch 1: constant 1 */
    } else {
        a = 0;  /* Branch 2: constant 0 */
    }
    
    /* Copy propagation chain */
    int b = a;  /* First copy */
    int c = b;  /* Second copy */
    int d = c;  /* Third copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (d == 0) {
        use(100);
        checksum += 100;
    } else {
        use(200);
        checksum += 200;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
void test_phi_long_chain_compare_one(int x) {
    int val;
    
    /* Different branch structure for phi */
    if (x & 1) {
        val = 1;
    } else if (x & 2) {
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
    
    /* Compare to 1 (not 0) */
    if (t5 == 1) {
        use(300);
        checksum += 300;
    } else {
        use(400);
        checksum += 400;
    }
}

/* Test 3: Phi in loop context */
__attribute__((noinline, noipa))
void test_phi_in_loop(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        int flag;
        
        /* Phi node inside loop */
        if (i & 1) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy chain */
        int f1 = flag;
        int f2 = f1;
        
        /* Conditional branch */
        if (f2 == 0) {
            total += i;
        } else {
            total -= i;
        }
    }
    
    checksum += total;
    use(total);
}

/* Test 4: Nested control flow with phi */
__attribute__((noinline, noipa))
void test_nested_phi(int x, int y) {
    int result;
    
    if (x > 0) {
        if (y > 0) {
            result = 1;
        } else {
            result = 0;
        }
    } else {
        result = 0;
    }
    
    /* Copy through multiple variables */
    int r1 = result;
    int r2 = r1;
    
    /* Compare to constant */
    if (r2 != 1) {  /* Using != instead of == */
        use(500);
        checksum += 500;
    } else {
        use(600);
        checksum += 600;
    }
}

/* Test 5: Phi with volatile input to prevent optimization */
__attribute__((noinline, noipa))
void test_phi_volatile(void) {
    int v;
    
    /* Use volatile to force real phi node */
    if (volatile_seed > 0) {
        v = 1;
    } else {
        v = 0;
    }
    
    /* Minimal copy chain */
    int w = v;
    
    /* Conditional */
    if (w == 0) {
        checksum += 700;
    } else {
        checksum += 800;
    }
}

/* Test 6: Switch-based phi with copy chain */
__attribute__((noinline, noipa))
void test_switch_phi(int x) {
    int code;
    
    switch (x % 4) {
        case 0: code = 1; break;
        case 1: code = 0; break;
        case 2: code = 1; break;
        default: code = 0; break;
    }
    
    /* Copy chain */
    int c1 = code;
    int c2 = c1;
    int c3 = c2;
    
    /* Compare to 0 */
    if (c3 == 0) {
        checksum += 900;
    } else {
        checksum += 1000;
    }
}

/* Test 7: Multiple phi nodes feeding into each other */
__attribute__((noinline, noipa))
void test_multi_phi(int x, int y) {
    int a, b;
    
    /* First phi */
    if (x > 0) {
        a = 1;
    } else {
        a = 0;
    }
    
    /* Second phi */
    if (y > 0) {
        b = 1;
    } else {
        b = 0;
    }
    
    /* Combine phis */
    int combined = a & b;
    
    /* Copy */
    int final = combined;
    
    /* Conditional */
    if (final == 1) {
        checksum += 1100;
    } else {
        checksum += 1200;
    }
}

/* Test 8: Do-while loop with phi */
__attribute__((noinline, noipa))
void test_dowhile_phi(int limit) {
    int i = 0;
    int sum = 0;
    
    do {
        int flag;
        
        /* Phi inside loop */
        if (i % 3 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy chain */
        int f = flag;
        int g = f;
        
        /* Conditional using the copy */
        if (g == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        i++;
    } while (i < limit);
    
    checksum += sum;
}

/* Main function that exercises all test cases */
int main(void) {
    /* Initialize with volatile to prevent constant folding */
    volatile_seed = 42;
    
    /* Run all test functions with different inputs */
    test_phi_copy_compare_zero(volatile_seed);
    test_phi_long_chain_compare_one(volatile_seed);
    test_phi_in_loop(10);
    test_nested_phi(volatile_seed, -volatile_seed);
    test_phi_volatile();
    test_switch_phi(volatile_seed);
    test_multi_phi(volatile_seed, volatile_seed / 2);
    test_dowhile_phi(8);
    
    /* Print checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy implementation of external functions to satisfy linker */
void use(int x) {
    checksum += x % 7;  /* Minimal side effect */
}

void use_ptr(void* p) {
    /* Minimal side effect */
    if (p) checksum += 1;
}
