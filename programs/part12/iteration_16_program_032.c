/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline,noipa))
int test_phi_copy_compare_zero(int x) {
    int val;
    volatile int v = g_seed;
    
    /* Create phi node with constants 0 and 1 */
    if (v > 0) {
        val = 0;  /* First constant */
    } else {
        val = 1;  /* Second constant */
    }
    
    /* Copy propagation chain */
    int a = val;   /* First copy */
    int b = a;     /* Second copy */
    int c = b;     /* Third copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (c == 0) {
        use(1);
        return 1;
    } else {
        use(0);
        return 0;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline,noipa))
int test_phi_long_chain_compare_one(int x) {
    int val;
    volatile int v = g_seed + x;
    
    /* Phi with three incoming edges simulated */
    if (v > 100) {
        val = 1;
    } else if (v > 50) {
        val = 0;
    } else {
        val = 1;  /* Another 1 for third edge */
    }
    
    /* Longer copy chain */
    int t1 = val;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    /* Compare to 1 */
    if (t5 == 1) {
        use(100);
        return 100;
    } else {
        use(200);
        return 200;
    }
}

/* Test 3: Phi in loop context */
__attribute__((noinline,noipa))
int test_phi_in_loop(int iterations) {
    volatile int v = g_seed;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int val;
        
        /* Phi node inside loop */
        if ((v + i) % 3 == 0) {
            val = 0;
        } else {
            val = 1;
        }
        
        /* Copy chain */
        int x = val;
        int y = x;
        
        /* Conditional branch */
        if (y == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    
    return sum;
}

/* Test 4: Nested phi nodes with copy propagation */
__attribute__((noinline,noipa))
int test_nested_phi_copy(int x) {
    volatile int v1 = g_seed;
    volatile int v2 = g_seed + 1;
    
    int val1, val2;
    
    /* First phi */
    if (v1 > 0) {
        val1 = 0;
    } else {
        val1 = 1;
    }
    
    /* Second phi */
    if (v2 < 0) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    
    /* Copy chains that merge */
    int a = val1;
    int b = val2;
    int c = (x > 0) ? a : b;
    int d = c;
    
    /* Conditional on the merged copy */
    if (d == 0) {
        use(999);
        return 999;
    } else {
        use(111);
        return 111;
    }
}

/* Test 5: Phi with boolean constants from function args */
__attribute__((noinline,noipa))
int test_phi_from_args(int flag1, int flag2) {
    int val;
    
    /* Phi based on function arguments (not known at compile time) */
    if (flag1) {
        val = 1;
    } else if (flag2) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Minimal copy chain */
    int tmp = val;
    
    /* Compare to 1 using != instead of == */
    if (tmp != 1) {  /* Should become tmp == 0 in GIMPLE */
        use(333);
        return 333;
    } else {
        use(444);
        return 444;
    }
}

/* Test 6: Switch-based phi with copy chain */
__attribute__((noinline,noipa))
int test_switch_phi(int x) {
    volatile int v = g_seed;
    int val;
    
    /* Switch creates multiple incoming edges to phi */
    switch (v % 4) {
        case 0: val = 0; break;
        case 1: val = 1; break;
        case 2: val = 0; break;
        default: val = 1; break;
    }
    
    /* Copy chain */
    int a = val;
    int b = a;
    int c = b;
    
    /* Conditional */
    if (c == 0) {
        return x * 2;
    } else {
        return x / 2;
    }
}

/* Main function that exercises all patterns */
int main() {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        g_seed = i * 17;
        
        checksum ^= test_phi_copy_compare_zero(i);
        checksum ^= test_phi_long_chain_compare_one(i);
        checksum ^= test_phi_in_loop(5 + (i % 3));
        checksum ^= test_nested_phi_copy(i);
        checksum ^= test_phi_from_args(i & 1, (i >> 1) & 1);
        checksum ^= test_switch_phi(i);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
}

void use_ptr(void* p) {
    /* Empty - just to prevent optimization */
}
