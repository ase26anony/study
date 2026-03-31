/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_basic_compare_zero(int x) {
    int val;
    volatile int v = g_seed;
    
    /* Create phi node with constants 0 and 1 */
    if (v > 0) {
        val = 0;  /* First phi argument */
    } else {
        val = 1;  /* Second phi argument */
    }
    
    /* Single copy assignment */
    int copy1 = val;
    
    /* Compare copy to constant 0 */
    if (copy1 == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_chain_compare_one(int x) {
    int val;
    volatile int v = g_seed + x;
    
    /* Phi with constants 1 and 0 (reversed) */
    if (v % 3 == 0) {
        val = 1;
    } else if (v % 3 == 1) {
        val = 0;
    } else {
        val = 1;  /* Third phi argument */
    }
    
    /* Chain of copy assignments */
    int a = val;
    int b = a;
    int c = b;
    int d = c;
    
    /* Compare final copy to constant 1 */
    if (d == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    volatile int v = g_seed;
    
    for (int i = 0; i < iterations; i++) {
        int flag;
        
        /* Phi node inside loop */
        if ((v + i) % 2 == 0) {
            flag = 0;
        } else {
            flag = 1;
        }
        
        /* Copy chain */
        int tmp1 = flag;
        int tmp2 = tmp1;
        
        /* Compare to 0 */
        if (tmp2 == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
    }
    
    use(sum);
    return sum;
}

/* Test 4: Nested phi nodes with != comparison */
__attribute__((noinline, noipa))
int test_nested_phi_not_equal(int x) {
    int val1, val2;
    volatile int v1 = g_seed;
    volatile int v2 = g_seed + 1;
    
    /* First phi */
    if (v1 > 10) {
        val1 = 0;
    } else {
        val1 = 1;
    }
    
    /* Second phi */
    if (v2 < 5) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    
    /* Combine phis */
    int combined = (val1 + val2) > 0 ? 1 : 0;
    
    /* Copy chain */
    int c1 = combined;
    int c2 = c1;
    
    /* Compare using != 0 (should become == 1 after optimization) */
    if (c2 != 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 5: Phi with multiple predecessors (switch-based) */
__attribute__((noinline, noipa))
int test_phi_switch(int x) {
    int result;
    volatile int v = g_seed + x;
    
    switch (v % 4) {
        case 0:
            result = 0;
            break;
        case 1:
            result = 1;
            break;
        case 2:
            result = 0;
            break;
        default:
            result = 1;
            break;
    }
    
    /* Long copy chain */
    int r1 = result;
    int r2 = r1;
    int r3 = r2;
    int r4 = r3;
    int r5 = r4;
    
    /* Compare to 1 */
    if (r5 == 1) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 6: Boolean phi from comparison */
__attribute__((noinline, noipa))
int test_bool_phi(int x) {
    volatile int v = g_seed;
    
    /* Create boolean value via comparison */
    int is_positive;
    if (v > 0) {
        is_positive = 1;
    } else {
        is_positive = 0;
    }
    
    /* Another boolean */
    int is_even;
    if ((x % 2) == 0) {
        is_even = 1;
    } else {
        is_even = 0;
    }
    
    /* Combine booleans - creates phi of phi results */
    int should_use = is_positive && is_even ? 1 : 0;
    
    /* Copy */
    int check = should_use;
    
    /* Compare to 0 */
    if (check == 0) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test 7: Recursive copy pattern */
__attribute__((noinline, noipa))
int test_recursive_copy(int x) {
    volatile int v = g_seed;
    int base;
    
    if (v % 7 == 0) {
        base = 1;
    } else {
        base = 0;
    }
    
    /* Chain of 10 copies */
    int c01 = base;
    int c02 = c01;
    int c03 = c02;
    int c04 = c03;
    int c05 = c04;
    int c06 = c05;
    int c07 = c06;
    int c08 = c07;
    int c09 = c08;
    int c10 = c09;
    
    /* Compare final copy to 1 */
    if (c10 == 1) {
        use(1100);
        return 11;
    } else {
        use(1200);
        return 12;
    }
}

/* Main driver that exercises all patterns */
int main() {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Call each test function multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        checksum += test_phi_basic_compare_zero(i);
        checksum += test_phi_chain_compare_one(i);
        checksum += test_phi_in_loop(5 + (i % 3));
        checksum += test_nested_phi_not_equal(i);
        checksum += test_phi_switch(i);
        checksum += test_bool_phi(i);
        checksum += test_recursive_copy(i);
        
        /* Modify seed to change execution paths */
        g_seed += i * 13;
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Also test with different optimization barriers */
    volatile int final_check = checksum;
    if (final_check > 1000) {
        printf("Patterns executed successfully\n");
    }
    
    return 0;
}

/* Dummy implementation of use() to satisfy linker */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Empty */
    (void)p;
}
