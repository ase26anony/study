/* test_autofdo_phi_patterns.c */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force different execution paths */
volatile int g_volatile_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_basic_compare_zero(int input) {
    int x;
    if (input > 0) {
        x = 1;  /* Will become phi argument 1 */
    } else {
        x = 0;  /* Will become phi argument 0 */
    }
    
    /* Copy chain */
    int a = x;      /* First copy */
    int b = a;      /* Second copy */
    
    /* Conditional comparing copy to 0 */
    if (b == 0) {   /* Should become: if (phi_result == 0) after copy propagation */
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
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
    
    /* Compare to 1 */
    if (t5 == 1) {  /* if (phi_result == 1) */
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi with three incoming edges (if-else if-else) */
__attribute__((noinline, noipa))
int test_phi_three_way(int input) {
    int z;
    if (input > 10) {
        z = 1;
    } else if (input < -10) {
        z = 0;
    } else {
        z = 1;  /* Same as first branch to test phi merging */
    }
    
    int copy1 = z;
    int copy2 = copy1;
    
    if (copy2 != 0) {  /* if (phi_result != 0) */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Phi in loop with copy chain */
__attribute__((noinline, noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    volatile int vol = g_volatile_seed; /* Prevent loop unrolling */
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        if (vol > 0) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        if (tmp2 == 0) {  /* if (phi_result == 0) */
            sum += i;
        } else {
            sum += i * 2;
        }
        
        /* Modify volatile to change path */
        vol = vol ^ 1;
    }
    return sum;
}

/* Test 5: Nested phi-copy-conditional pattern */
__attribute__((noinline, noipa))
int test_nested_phi(int input1, int input2) {
    int result;
    
    /* Outer phi */
    if (input1 > 0) {
        /* Inner phi pattern */
        int inner;
        if (input2 > 0) {
            inner = 1;
        } else {
            inner = 0;
        }
        
        int copy_inner = inner;
        if (copy_inner == 1) {
            result = 100;
        } else {
            result = 200;
        }
    } else {
        result = 300;
    }
    
    /* Final conditional on result (not from phi, but ensures all code is used) */
    if (result > 150) {
        use(700);
    } else {
        use(800);
    }
    
    return result;
}

/* Test 6: Phi with boolean constants from different types of comparisons */
__attribute__((noinline, noipa))
int test_phi_mixed_sources(int a, int b) {
    int flag;
    
    /* Multiple ways to get 0/1 */
    if (a > b) {
        flag = 1;
    } else {
        flag = (a == b) ? 1 : 0;  /* Another phi might be created here */
    }
    
    /* Multi-step copy chain */
    int f1 = flag;
    int f2 = f1;
    int f3 = f2;
    
    /* Compare to 0 */
    if (f3 == 0) {
        return a + b;
    } else {
        return a - b;
    }
}

/* Test 7: Pointer-based conditional to force GIMPLE_COND */
__attribute__((noinline, noipa))
int test_phi_with_pointers(int cond) {
    int x;
    if (cond) {
        x = 1;
    } else {
        x = 0;
    }
    
    int y = x;
    int z = y;
    
    /* This should generate GIMPLE_COND with SSA_NAME == 0 */
    if (z) {  /* Equivalent to if (z != 0) */
        use_ptr(&x);
        return 1;
    }
    use_ptr(&y);
    return 0;
}

/* Dummy implementation of external functions to avoid linker errors */
void use(int x) {
    /* Do nothing - just prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Do nothing - just prevent optimization */
    static volatile void* sink;
    sink = p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent constant propagation */
    volatile int seed = g_volatile_seed;
    
    /* Run all tests with different inputs to exercise all paths */
    checksum += test_phi_basic_compare_zero(seed);
    checksum += test_phi_basic_compare_zero(-seed);
    
    checksum += test_phi_long_chain_compare_one(seed);
    checksum += test_phi_long_chain_compare_one(seed + 1);
    
    checksum += test_phi_three_way(20);
    checksum += test_phi_three_way(-20);
    checksum += test_phi_three_way(0);
    
    checksum += test_phi_in_loop(5);
    
    checksum += test_nested_phi(1, 1);
    checksum += test_nested_phi(1, -1);
    checksum += test_nested_phi(-1, 1);
    
    checksum += test_phi_mixed_sources(10, 5);
    checksum += test_phi_mixed_sources(5, 10);
    checksum += test_phi_mixed_sources(5, 5);
    
    checksum += test_phi_with_pointers(1);
    checksum += test_phi_with_pointers(0);
    
    printf("Checksum: %d\n", checksum);
    
    /* Also run with profile generation if compiled with -fprofile-generate */
    if (seed == 0) {
        /* Force execution of rarely taken paths */
        for (int i = 0; i < 100; i++) {
            test_phi_in_loop(i % 3);
        }
    }
    
    return 0;
}
