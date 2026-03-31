/* test_autofdo_phi_conditional.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Simple phi with 0/1 constants, 1-level copy chain */
__attribute__((noinline, noipa))
int test_phi_copy_compare_zero(void) {
    volatile int cond = g_seed & 1;
    int result = 0;
    
    /* Create phi node with constants 0 and 1 */
    int phi_val;
    if (cond) {
        phi_val = 1;  /* Constant 1 */
    } else {
        phi_val = 0;  /* Constant 0 */
    }
    
    /* Single copy assignment chain */
    int copy1 = phi_val;
    
    /* Conditional branch comparing copy to constant 0 */
    if (copy1 == 0) {
        use(100);
        result = 1;
    } else {
        use(200);
        result = 2;
    }
    
    return result;
}

/* Test function 2: Multi-level copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_multi_copy_compare_one(void) {
    volatile int cond = (g_seed >> 1) & 1;
    int result = 0;
    
    /* Phi with constants */
    int phi_val;
    if (cond) {
        phi_val = 0;
    } else {
        phi_val = 1;
    }
    
    /* Multi-level copy chain */
    int a = phi_val;
    int b = a;
    int c = b;
    int d = c;
    
    /* Compare final copy to constant 1 */
    if (d == 1) {
        use(300);
        result = 3;
    } else {
        use(400);
        result = 4;
    }
    
    return result;
}

/* Test function 3: Phi with 3 incoming edges (switch-like) */
__attribute__((noinline, noipa))
int test_phi_three_way(void) {
    volatile int selector = g_seed % 3;
    int phi_val;
    
    /* Three-way phi */
    if (selector == 0) {
        phi_val = 0;
    } else if (selector == 1) {
        phi_val = 1;
    } else {
        phi_val = 0;  /* Another 0 to create interesting pattern */
    }
    
    /* Copy chain */
    int x = phi_val;
    int y = x;
    
    /* Compare to 0 */
    if (y != 0) {  /* Note: != instead of == */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Loop with phi-copy-conditional inside */
__attribute__((noinline, noipa))
int test_loop_phi_pattern(void) {
    volatile int iter_count = (g_seed % 5) + 2;
    int sum = 0;
    
    for (int i = 0; i < iter_count; i++) {
        volatile int inner_cond = (g_seed + i) & 1;
        int phi_val;
        
        /* Phi node inside loop */
        if (inner_cond) {
            phi_val = 1;
        } else {
            phi_val = 0;
        }
        
        /* Copy chain */
        int tmp1 = phi_val;
        int tmp2 = tmp1;
        
        /* Conditional based on copy */
        if (tmp2 == 1) {
            sum += 10;
        } else {
            sum += 20;
        }
    }
    
    use(sum);
    return sum;
}

/* Test function 5: Nested conditionals creating complex phi */
__attribute__((noinline, noipa))
int test_nested_phi(void) {
    volatile int cond1 = g_seed & 1;
    volatile int cond2 = (g_seed >> 1) & 1;
    int phi_val;
    
    /* Complex phi with nested conditions */
    if (cond1) {
        if (cond2) {
            phi_val = 1;
        } else {
            phi_val = 0;
        }
    } else {
        phi_val = 0;
    }
    
    /* Longer copy chain */
    int v1 = phi_val;
    int v2 = v1;
    int v3 = v2;
    int v4 = v3;
    int v5 = v4;
    
    /* Compare to 0 */
    if (v5 == 0) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test function 6: Phi with boolean constants from function args */
__attribute__((noinline, noipa))
int test_phi_from_args(int arg1, int arg2) {
    int phi_val;
    
    /* Phi based on arguments (not known at compile time) */
    if (arg1 > arg2) {
        phi_val = 1;
    } else {
        phi_val = 0;
    }
    
    /* Copy and compare */
    int copy = phi_val;
    
    if (copy == 1) {
        return 9;
    } else {
        return 10;
    }
}

/* Dummy use function to prevent optimization */
void use(int x) {
    /* Empty - just to create side effect */
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Call all test functions */
    checksum += test_phi_copy_compare_zero();
    checksum += test_multi_copy_compare_one();
    checksum += test_phi_three_way();
    checksum += test_loop_phi_pattern();
    checksum += test_nested_phi();
    checksum += test_phi_from_args(g_seed, g_seed / 2);
    
    /* Run multiple times with different seeds */
    for (int i = 0; i < 3; i++) {
        g_seed = i * 17;
        checksum += test_phi_copy_compare_zero();
        checksum += test_multi_copy_compare_one();
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
