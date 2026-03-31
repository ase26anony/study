/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test function 1: Simple phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_simple_copy_compare_zero(volatile int cond) {
    int val;
    if (cond > 0) {
        val = 1;  /* Branch 1: constant 1 */
    } else {
        val = 0;  /* Branch 2: constant 0 */
    }
    /* Creates phi node: val = PHI<1, 0> */
    
    int copy1 = val;  /* GIMPLE_ASSIGN with SSA_NAME rhs */
    
    if (copy1 == 0) {  /* GIMPLE_COND comparing SSA_NAME to constant 0 */
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Multi-copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_multi_copy_compare_one(volatile int cond) {
    int val;
    if (cond & 1) {
        val = 1;
    } else {
        val = 0;
    }
    /* val = PHI<1, 0> */
    
    int a = val;    /* First copy */
    int b = a;      /* Second copy - GIMPLE_ASSIGN with SSA_NAME */
    int c = b;      /* Third copy - creates chain */
    int d = c;      /* Fourth copy */
    
    if (d == 1) {   /* Compare final copy to constant 1 */
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test function 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
int test_phi_three_way(volatile int cond) {
    int val;
    if (cond > 10) {
        val = 1;
    } else if (cond > 5) {
        val = 0;
    } else {
        val = 1;  /* Different constant but same as first branch */
    }
    /* val = PHI<1, 0, 1> with three incoming edges */
    
    int tmp = val;
    int tmp2 = tmp;
    
    if (tmp2 != 0) {  /* Compare with != 0 */
        use(500);
        return 5;
    }
    use(600);
    return 6;
}

/* Test function 4: Phi in loop context */
__attribute__((noinline, noipa))
int test_phi_in_loop(volatile int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        /* val = PHI<1, 0> inside loop */
        
        int copy = val;
        int copy2 = copy;
        
        if (copy2 == 0) {
            sum += i * 2;
        } else {
            sum += i * 3;
        }
    }
    return sum;
}

/* Test function 5: Nested phi-copy pattern */
__attribute__((noinline, noipa))
int test_nested_phi_copy(volatile int cond1, volatile int cond2) {
    int val1, val2;
    
    if (cond1 > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    if (cond2 > 0) {
        val2 = val1;  /* Uses phi result */
    } else {
        val2 = 0;
    }
    /* val2 = PHI<val1(phi result), 0> */
    
    int chain1 = val2;
    int chain2 = chain1;
    
    if (chain2 == 1) {
        use(700);
        return 7;
    }
    use(800);
    return 8;
}

/* Test function 6: Copy chain with arithmetic that doesn't break SSA chain */
__attribute__((noinline, noipa))
int test_phi_copy_with_simple_arith(volatile int cond) {
    int val;
    if (cond > 100) {
        val = 1;
    } else {
        val = 0;
    }
    
    int a = val;
    int b = a + 0;  /* Simple arithmetic that becomes copy in GIMPLE */
    int c = b;
    
    if (c == 0) {
        use(900);
        return 9;
    }
    use(1000);
    return 10;
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

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = g_seed;
    if (seed == 0) {
        seed = 1;
    }
    
    /* Call test functions with different conditions */
    checksum += test_phi_simple_copy_compare_zero(seed);
    checksum += test_phi_multi_copy_compare_one(seed + 1);
    checksum += test_phi_three_way(seed + 2);
    checksum += test_phi_in_loop(seed + 3);
    checksum += test_nested_phi_copy(seed + 4, seed + 5);
    checksum += test_phi_copy_with_simple_arith(seed + 6);
    
    printf("Checksum: %d\n", checksum);
    
    /* Force execution of all paths by calling with different seeds */
    for (int i = 0; i < 10; i++) {
        test_phi_simple_copy_compare_zero(i);
        test_phi_multi_copy_compare_one(i);
        test_phi_three_way(i);
        test_phi_in_loop(i);
        test_nested_phi_copy(i, i+1);
        test_phi_copy_with_simple_arith(i);
    }
    
    return 0;
}
