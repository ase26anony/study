/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_zero(volatile int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Branch 1: constant 1 */
    } else {
        x = 0;  /* Branch 2: constant 0 */
    }
    /* Copy chain: phi -> a -> b -> c */
    int a = x;
    int b = a;
    int c = b;
    
    /* Conditional comparing copy chain result to 0 */
    if (c == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(volatile int cond) {
    int val;
    if (cond & 1) {
        val = 0;
    } else {
        val = 1;
    }
    /* Longer copy chain */
    int t1 = val;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    if (t5 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
int test_phi_three_way(volatile int cond) {
    int result;
    if (cond == 1) {
        result = 1;
    } else if (cond == 2) {
        result = 0;
    } else {
        result = 1;  /* Same constant as first branch */
    }
    
    /* Copy chain */
    int tmp = result;
    int final = tmp;
    
    if (final == 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Nested loops with phi-copy-conditional pattern */
__attribute__((noinline, noipa))
int test_loop_phi_pattern(volatile int iter_count) {
    int sum = 0;
    volatile int loop_cond = iter_count;
    
    for (int i = 0; i < (loop_cond & 3) + 1; i++) {
        int flag;
        if (i & 1) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy chain inside loop */
        int f1 = flag;
        int f2 = f1;
        
        if (f2 == 1) {
            sum += 7;
            use(sum);
        } else {
            sum += 11;
            use(sum);
        }
    }
    return sum;
}

/* Test 5: Phi with copy chain using pointer indirection */
__attribute__((noinline, noipa))
int test_phi_with_indirect(volatile int cond) {
    int base;
    if (cond > 10) {
        base = 1;
    } else {
        base = 0;
    }
    
    /* Copy through pointer */
    int copy1 = base;
    int *ptr = &copy1;
    int copy2 = *ptr;
    
    if (copy2 == 0) {
        use(700);
        return 7;
    }
    use(800);
    return 8;
}

/* Test 6: Multiple phi nodes feeding into each other */
__attribute__((noinline, noipa))
int test_cascaded_phis(volatile int c1, volatile int c2) {
    int x, y;
    
    /* First phi */
    if (c1 > 0) {
        x = 1;
    } else {
        x = 0;
    }
    
    /* Second phi dependent on first */
    if (c2 > 0) {
        y = x;  /* Could be 0 or 1 */
    } else {
        y = 1 - x;  /* Opposite of x */
    }
    
    /* Copy chain */
    int z = y;
    int w = z;
    
    if (w == 1) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test 7: Switch statement creating phi with constants */
__attribute__((noinline, noipa))
int test_switch_phi(volatile int selector) {
    int value;
    
    switch (selector & 3) {
        case 0: value = 0; break;
        case 1: value = 1; break;
        case 2: value = 0; break;
        default: value = 1; break;
    }
    
    int copy = value;
    
    if (copy == 0) {
        use(1100);
        return 11;
    }
    use(1200);
    return 12;
}

/* Main function that exercises all patterns */
int main() {
    volatile int seed = g_seed;
    int checksum = 0;
    
    /* Call each test function multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        checksum += test_phi_copy_compare_zero(seed + i);
        checksum += test_phi_long_chain_compare_one(seed + i * 2);
        checksum += test_phi_three_way(seed + i * 3);
        checksum += test_loop_phi_pattern(seed + i);
        checksum += test_phi_with_indirect(seed + i * 5);
        checksum += test_cascaded_phis(seed + i, seed + i + 1);
        checksum += test_switch_phi(seed + i * 7);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
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
