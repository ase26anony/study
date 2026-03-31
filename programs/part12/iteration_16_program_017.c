/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime decisions */
volatile int g_seed = 0;

/* Test function 1: Simple phi with 0/1 constants, short copy chain */
__attribute__((noinline, noipa))
int test_phi_simple_copy(int input) {
    int val;
    if (input > 0) {
        val = 1;  /* Branch 1: constant 1 */
    } else {
        val = 0;  /* Branch 2: constant 0 */
    }
    
    /* Copy chain: phi -> a -> b -> c */
    int a = val;
    int b = a;
    int c = b;
    
    /* Conditional branch comparing to 0 */
    if (c == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain(int input) {
    int val;
    if (input % 2 == 0) {
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
    
    /* Compare to 1 */
    if (t5 == 1) {
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
    int val;
    if (input > 10) {
        val = 1;
    } else if (input < -10) {
        val = 0;
    } else {
        val = 1;  /* Same as first branch but different path */
    }
    
    /* Copy through multiple variables */
    int x = val;
    int y = x;
    
    /* Compare to 0 with != operator */
    if (y != 0) {
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    volatile int vol = g_seed; /* Prevent loop unrolling */
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        if (vol + i > 5) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional inside loop */
        if (tmp2 == 1) {
            sum += i * 2;
        } else {
            sum += i;
        }
    }
    return sum;
}

/* Test function 5: Nested phi-copy pattern */
__attribute__((noinline, noipa))
int test_nested_phi(int input1, int input2) {
    int val1, val2;
    
    /* First phi */
    if (input1 > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    /* Second phi */
    if (input2 > 0) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    
    /* Copy chains */
    int a = val1;
    int b = val2;
    int c = a;
    int d = b;
    
    /* Two conditionals */
    int result = 0;
    if (c == 0) {
        result += 10;
    }
    if (d == 1) {
        result += 20;
    }
    
    use(result);
    return result;
}

/* Test function 6: Phi with boolean operations in branches */
__attribute__((noinline, noipa))
int test_phi_with_bool_ops(int x, int y) {
    int val;
    /* More complex condition to prevent optimization */
    if ((x > y) && (x < 100)) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Multi-step copy */
    int step1 = val;
    int step2 = step1;
    int step3 = step2;
    
    /* Final comparison */
    if (step3 == 0) {
        return x + y;
    } else {
        return x - y;
    }
}

/* Dummy implementation of use() to avoid linker errors */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    static volatile void* sink;
    sink = p;
}

int main(void) {
    int checksum = 0;
    volatile int seed = g_seed;
    
    /* Call test functions with different inputs to exercise all paths */
    checksum += test_phi_simple_copy(seed);
    checksum += test_phi_simple_copy(-seed);
    
    checksum += test_phi_long_chain(seed);
    checksum += test_phi_long_chain(seed + 1);
    
    checksum += test_phi_three_way(seed);
    checksum += test_phi_three_way(seed - 20);
    checksum += test_phi_three_way(seed + 20);
    
    checksum += test_phi_in_loop(5 + (seed & 3));
    
    checksum += test_nested_phi(seed, -seed);
    checksum += test_nested_phi(-seed, seed);
    
    checksum += test_phi_with_bool_ops(seed, seed * 2);
    checksum += test_phi_with_bool_ops(seed * 3, seed);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
