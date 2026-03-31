/* test_autofdo_phi_conditional.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_zero(volatile int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Will become phi argument 1 */
    } else {
        x = 0;  /* Will become phi argument 0 */
    }
    
    /* Copy propagation chain */
    int a = x;  /* First copy */
    int b = a;  /* Second copy */
    int c = b;  /* Third copy - final in chain */
    
    /* Conditional branch comparing copy chain result to 0 */
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
int test_phi_long_chain_compare_one(volatile int cond1, volatile int cond2) {
    int y;
    if (cond1 > 0) {
        y = 1;
    } else if (cond2 > 0) {
        y = 0;
    } else {
        y = 1;  /* Three-way phi with constants 0 and 1 */
    }
    
    /* Longer copy chain */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;  /* Fifth copy */
    
    /* Compare to 1 */
    if (t5 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    volatile int loop_cond = iterations;
    
    for (int i = 0; i < loop_cond && i < 10; i++) {
        int z;
        if (i % 2 == 0) {
            z = 1;
        } else {
            z = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = z;
        int tmp2 = tmp1;
        
        /* Conditional with copy result */
        if (tmp2 == 0) {
            sum += i * 2;
            use(sum);
        } else {
            sum += i * 3;
            use(sum);
        }
    }
    return sum;
}

/* Test 4: Nested phi nodes with copy propagation */
__attribute__((noinline, noipa))
int test_nested_phi_copy(volatile int a, volatile int b) {
    int val1, val2;
    
    /* First phi */
    if (a > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    /* Second phi dependent on first */
    if (b > 0) {
        val2 = val1;  /* Could be 0 or 1 */
    } else {
        val2 = (val1 == 0) ? 1 : 0;  /* Still 0 or 1 */
    }
    
    /* Copy chain */
    int copy1 = val2;
    int copy2 = copy1;
    
    /* Multiple comparisons */
    int result = 0;
    if (copy2 == 0) {
        result = 10;
        use(result);
    }
    if (copy2 == 1) {
        result = 20;
        use(result);
    }
    
    return result;
}

/* Test 5: Phi with arithmetic that still yields 0/1 */
__attribute__((noinline, noipa))
int test_phi_arithmetic_bool(volatile int x, volatile int y) {
    int cmp_result;
    if (x > y) {
        cmp_result = 1;
    } else {
        cmp_result = 0;
    }
    
    /* Arithmetic that preserves 0/1 nature */
    int adjusted = cmp_result * 2 - cmp_result;  /* Still 0 or 1 */
    
    /* Copy chain */
    int final = adjusted;
    
    /* Compare to 0 using != operator */
    if (final != 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 6: Switch-based phi with constants */
__attribute__((noinline, noipa))
int test_switch_phi(volatile int selector) {
    int flag;
    switch (selector % 3) {
        case 0: flag = 0; break;
        case 1: flag = 1; break;
        case 2: flag = 0; break;  /* Still 0 or 1 */
        default: flag = 1; break;
    }
    
    /* Minimal copy chain */
    int f = flag;
    
    /* Compare to 1 */
    if (f == 1) {
        use(700);
        return 7;
    }
    use(800);
    return 8;
}

/* Main function that exercises all patterns */
int main() {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int v1 = 1;
    volatile int v2 = 0;
    volatile int v3 = 2;
    volatile int v4 = -1;
    volatile int v5 = 3;
    
    /* Run all tests multiple times with different conditions */
    for (int i = 0; i < 3; i++) {
        checksum += test_phi_copy_compare_zero(v1 + i);
        checksum += test_phi_long_chain_compare_one(v2 + i, v3 - i);
        checksum += test_phi_in_loop(v4 + i + 4);  /* Ensure loop runs */
        checksum += test_nested_phi_copy(v5 + i, v1 - i);
        checksum += test_phi_arithmetic_bool(v2 + i, v3 + i);
        checksum += test_switch_phi(v4 + i + 10);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Also test edge cases */
    g_seed = checksum;
    volatile int edge = g_seed;
    
    /* One more test with the global volatile */
    checksum += test_phi_copy_compare_zero(edge);
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}

/* Dummy implementation of external functions to allow linking */
void use(int x) {
    /* Empty - just to prevent optimization */
    static int dummy;
    dummy += x;
}

void use_ptr(int* p) {
    /* Empty */
    static int dummy;
    if (p) dummy += *p;
}
