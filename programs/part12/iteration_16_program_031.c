/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime decisions */
volatile int g_seed = 0;

/* Test function 1: Simple phi with 0/1 constants, short copy chain */
__attribute__((noinline, noipa))
int test_phi_simple_copy(int input) {
    int result;
    
    /* Create phi node with constants 0 and 1 */
    if (input > 0) {
        result = 1;  /* Constant 1 */
    } else {
        result = 0;  /* Constant 0 */
    }
    
    /* Create copy propagation chain */
    int a = result;  /* First copy */
    int b = a;       /* Second copy */
    
    /* Conditional branch comparing to constant 0 */
    if (b == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to constant 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain(int input) {
    int val;
    
    /* Different branch structure for phi */
    if (input & 1) {
        val = 1;
    } else if (input & 2) {
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
    
    /* Compare to constant 1 */
    if (t5 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with volatile-based decision */
__attribute__((noinline, noipa))
int test_phi_volatile(void) {
    int x;
    
    /* Use volatile to prevent constant propagation */
    volatile int v = g_seed;
    if (v > 100) {
        x = 0;
    } else {
        x = 1;
    }
    
    /* Copy chain with pointer indirection */
    int y = x;
    int z = y;
    
    /* Conditional with != comparison */
    if (z != 0) {  /* Equivalent to z == 1 for boolean values */
        use(500);
        return 5;
    }
    
    use(600);
    return 6;
}

/* Test function 4: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    volatile int v = g_seed;
    
    for (int i = 0; i < iterations; i++) {
        int flag;
        
        /* Phi node inside loop */
        if (v + i > 50) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy chain */
        int f1 = flag;
        int f2 = f1;
        
        /* Conditional branch inside loop */
        if (f2 == 1) {
            sum += i * 2;
        } else {
            sum += i;
        }
    }
    
    use(sum);
    return sum;
}

/* Test function 5: Nested phi nodes */
__attribute__((noinline, noipa))
int test_nested_phi(int a, int b) {
    int inner;
    
    if (a > b) {
        inner = 1;
    } else {
        inner = 0;
    }
    
    int copy1 = inner;
    
    /* Another phi depending on first */
    int outer;
    if (copy1 == 0) {
        outer = 1;
    } else {
        outer = 0;
    }
    
    int copy2 = outer;
    
    if (copy2 == 1) {
        use(700);
        return 7;
    }
    
    use(800);
    return 8;
}

/* Test function 6: Multiple comparisons from same phi */
__attribute__((noinline, noipa))
int test_multi_compare(int x) {
    int choice;
    
    if (x % 3 == 0) {
        choice = 1;
    } else {
        choice = 0;
    }
    
    int c1 = choice;
    int c2 = c1;
    
    /* Multiple comparisons to trigger different edges */
    if (c2 == 0) {
        use(900);
    }
    
    if (c2 == 1) {
        use(901);
    }
    
    return choice;
}

/* Test function 7: Phi with more than two incoming edges */
__attribute__((noinline, noipa))
int test_multiway_phi(int mode) {
    int value;
    
    switch (mode % 4) {
        case 0: value = 0; break;
        case 1: value = 1; break;
        case 2: value = 0; break;
        case 3: value = 1; break;
    }
    
    int v1 = value;
    int v2 = v1;
    int v3 = v2;
    
    if (v3 == 0) {
        return 20;
    } else {
        return 21;
    }
}

/* Dummy implementation of use() to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(int *p) {
    static volatile int sink;
    if (p) sink = *p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Call test functions with different inputs */
    checksum += test_phi_simple_copy(g_seed);
    checksum += test_phi_long_chain(g_seed);
    checksum += test_phi_volatile();
    checksum += test_phi_in_loop(10);
    checksum += test_nested_phi(5, 3);
    checksum += test_nested_phi(2, 7);
    checksum += test_multi_compare(g_seed);
    checksum += test_multiway_phi(g_seed);
    
    /* Vary the seed and run again */
    g_seed = 123;
    checksum += test_phi_simple_copy(g_seed);
    checksum += test_phi_long_chain(g_seed);
    checksum += test_phi_volatile();
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
