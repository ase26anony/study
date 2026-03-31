/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Simple phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_chain_compare_zero(int input) {
    int x;
    if (input > 0) {
        x = 1;  /* Branch 1: constant 1 */
    } else {
        x = 0;  /* Branch 2: constant 0 */
    }
    
    /* Copy propagation chain */
    int a = x;  /* First copy */
    int b = a;  /* Second copy */
    int c = b;  /* Third copy */
    int d = c;  /* Fourth copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (d == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(int input) {
    int y;
    if (input % 2 == 0) {
        y = 0;
    } else {
        y = 1;
    }
    
    /* Longer copy chain */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    int t6 = t5;
    
    /* Compare to 1 (not equal) */
    if (t6 != 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
int test_phi_three_way(int input) {
    int z;
    if (input < -10) {
        z = 0;
    } else if (input < 10) {
        z = 1;
    } else {
        z = 0;  /* Another 0 to create interesting pattern */
    }
    
    /* Minimal copy chain */
    int copy = z;
    
    if (copy == 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Nested control flow with phi */
__attribute__((noinline, noipa))
int test_nested_phi(int input) {
    int result;
    
    if (input > 100) {
        if (input < 200) {
            result = 1;
        } else {
            result = 0;
        }
    } else {
        if (input > 50) {
            result = 1;
        } else {
            result = 0;
        }
    }
    
    int r1 = result;
    int r2 = r1;
    
    if (r2 == 1) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test function 5: Loop with phi inside */
__attribute__((noinline, noipa))
int test_loop_phi(int iterations) {
    int sum = 0;
    volatile int vol = g_seed; /* Prevent loop unrolling */
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int flag;
        if (i % 3 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy chain inside loop */
        int f1 = flag;
        int f2 = f1;
        
        if (f2 == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
    }
    
    use(sum);
    return sum;
}

/* Test function 6: Multiple phi nodes feeding each other */
__attribute__((noinline, noipa))
int test_multi_phi(int input1, int input2) {
    int val1, val2;
    
    if (input1 > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    if (input2 > 0) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    
    /* Combine two phi results */
    int combined = val1 & val2;
    int c1 = combined;
    int c2 = c1;
    
    if (c2 == 0) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test function 7: Switch-based phi */
__attribute__((noinline, noipa))
int test_switch_phi(int input) {
    int sw;
    
    switch (input % 4) {
        case 0: sw = 1; break;
        case 1: sw = 0; break;
        case 2: sw = 1; break;
        case 3: sw = 0; break;
        default: sw = 0; break;
    }
    
    int s1 = sw;
    int s2 = s1;
    int s3 = s2;
    
    if (s3 == 1) {
        use(1100);
        return 11;
    } else {
        use(1200);
        return 12;
    }
}

/* Dummy implementation of use() to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Empty */
    (void)p;
}

int main() {
    int checksum = 0;
    volatile int seed = g_seed;
    
    /* Call test functions with varying inputs to exercise different paths */
    checksum += test_phi_copy_chain_compare_zero(seed);
    checksum += test_phi_copy_chain_compare_zero(-seed);
    
    checksum += test_phi_long_chain_compare_one(seed);
    checksum += test_phi_long_chain_compare_one(seed + 1);
    
    checksum += test_phi_three_way(seed - 20);
    checksum += test_phi_three_way(seed);
    checksum += test_phi_three_way(seed + 20);
    
    checksum += test_nested_phi(seed + 30);
    checksum += test_nested_phi(seed + 150);
    checksum += test_nested_phi(seed - 50);
    
    checksum += test_loop_phi(5 + (seed & 0x3));
    
    checksum += test_multi_phi(seed, -seed);
    checksum += test_multi_phi(-seed, seed);
    
    checksum += test_switch_phi(seed);
    checksum += test_switch_phi(seed + 1);
    checksum += test_switch_phi(seed + 2);
    checksum += test_switch_phi(seed + 3);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
