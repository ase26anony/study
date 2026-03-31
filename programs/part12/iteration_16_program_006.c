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
int test_phi_copy_compare_zero(int input) {
    int val;
    
    /* Create phi node with constants 0 and 1 */
    if (input > 0) {
        val = 1;  /* Constant 1 */
    } else {
        val = 0;  /* Constant 0 */
    }
    
    /* Create copy propagation chain */
    int a = val;   /* First copy */
    int b = a;     /* Second copy */
    int c = b;     /* Third copy */
    int d = c;     /* Fourth copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (d == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(int input) {
    int val;
    
    /* Different branch structure for phi */
    if (input & 1) {
        val = 0;
    } else if (input & 2) {
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
    int t6 = t5;
    
    /* Compare to 1 */
    if (t6 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi with volatile input, compare to 0 with != */
__attribute__((noinline, noipa))
int test_phi_volatile_compare(int* ptr) {
    int val;
    volatile int v = *ptr;  /* Volatile read */
    
    if (v > 10) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Copy chain */
    int x = val;
    int y = x;
    
    /* Compare with != 0 */
    if (y != 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    volatile int v = g_seed;
    
    for (int i = 0; i < iterations; i++) {
        int val;
        
        /* Phi node inside loop */
        if ((i + v) & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional branch */
        if (tmp2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    
    return sum;
}

/* Test 5: Multi-way phi (more than 2 incoming edges) */
__attribute__((noinline, noipa))
int test_multiway_phi(int input) {
    int val;
    
    switch (input % 4) {
        case 0: val = 0; break;
        case 1: val = 1; break;
        case 2: val = 0; break;
        case 3: val = 1; break;
    }
    
    /* Copy chain */
    int a = val;
    int b = a;
    int c = b;
    
    /* Compare to 1 */
    if (c == 1) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test 6: Nested control flow with phi */
__attribute__((noinline, noipa))
int test_nested_phi(int input) {
    int val;
    
    if (input > 100) {
        if (input < 200) {
            val = 1;
        } else {
            val = 0;
        }
    } else {
        if (input > 50) {
            val = 1;
        } else {
            val = 0;
        }
    }
    
    /* Minimal copy chain */
    int v = val;
    
    /* Compare to 0 */
    if (v == 0) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test 7: Phi with pointer copy chain */
__attribute__((noinline, noipa))
int test_phi_pointer(int input) {
    int val;
    
    if (input % 3 == 0) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Copy through different variable types */
    int a = val;
    short b = a;
    int c = b;
    char d = c;
    int e = d;
    
    /* Compare to 1 */
    if (e == 1) {
        use(1100);
        return 11;
    } else {
        use(1200);
        return 12;
    }
}

/* Main function that exercises all patterns */
int main() {
    int checksum = 0;
    volatile int seed = g_seed;
    
    /* Call each test function multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        checksum += test_phi_copy_compare_zero(seed + i);
        checksum += test_phi_long_chain_compare_one(seed + i * 2);
        
        int temp = seed + i * 3;
        checksum += test_phi_volatile_compare(&temp);
        
        checksum += test_phi_in_loop(5 + (i % 3));
        checksum += test_multiway_phi(seed + i * 7);
        checksum += test_nested_phi(seed + i * 11);
        checksum += test_phi_pointer(seed + i * 13);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Also test with edge cases */
    checksum += test_phi_copy_compare_zero(0);
    checksum += test_phi_copy_compare_zero(1);
    checksum += test_phi_long_chain_compare_one(0);
    checksum += test_phi_long_chain_compare_one(1);
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy implementation of external functions to allow linking */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Empty - just to prevent optimization */
    static volatile void* sink;
    sink = p;
}
