/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime decisions */
volatile int g_seed = 0;

/* Test function 1: Simple phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_simple_compare_zero(int input) {
    int x;
    if (input > 0) {
        x = 1;  /* Branch 1: constant 1 */
    } else {
        x = 0;  /* Branch 2: constant 0 */
    }
    
    /* Single copy propagation */
    int y = x;
    
    /* Conditional comparing copy to 0 */
    if (y == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test function 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_long_chain_compare_one(int input) {
    int a;
    if (input & 1) {
        a = 0;  /* Constant 0 */
    } else {
        a = 1;  /* Constant 1 */
    }
    
    /* Multi-step copy propagation chain */
    int b = a;
    int c = b;
    int d = c;
    int e = d;
    
    /* Compare final copy to 1 */
    if (e == 1) {
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
    if (input < 0) {
        val = 0;
    } else if (input == 0) {
        val = 1;
    } else {
        val = 0;  /* Same as first branch to create interesting phi */
    }
    
    /* Copy chain */
    int tmp1 = val;
    int tmp2 = tmp1;
    
    /* Compare to 0 */
    if (tmp2 != 0) {  /* Using != instead of == */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test function 4: Nested control flow with volatile */
__attribute__((noinline, noipa))
int test_phi_with_loop(volatile int* counter) {
    int sum = 0;
    int limit = (*counter) & 3;  /* 0-3 iterations */
    
    for (int i = 0; i < limit; i++) {
        int flag;
        if (i & 1) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy through multiple temps */
        int f1 = flag;
        int f2 = f1;
        
        /* Conditional inside loop */
        if (f2 == 1) {
            sum += 10;
        } else {
            sum += 20;
        }
    }
    
    /* Final conditional based on sum */
    int copy_sum = sum;
    if (copy_sum > 25) {
        use(700);
        return 7;
    } else {
        use(800);
        return 8;
    }
}

/* Test function 5: Multiple phis feeding each other */
__attribute__((noinline, noipa))
int test_cascading_phis(int input1, int input2) {
    int x, y;
    
    /* First phi */
    if (input1 > 0) {
        x = 1;
    } else {
        x = 0;
    }
    
    /* Second phi (independent) */
    if (input2 > 0) {
        y = 1;
    } else {
        y = 0;
    }
    
    /* Combine them */
    int combined = x & y;  /* Creates another phi-like value */
    int copy1 = combined;
    int copy2 = copy1;
    
    /* Compare to 0 */
    if (copy2 == 0) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test function 6: Switch statement creating phi */
__attribute__((noinline, noipa))
int test_phi_from_switch(int input) {
    int result;
    
    switch (input & 3) {
        case 0: result = 0; break;
        case 1: result = 1; break;
        case 2: result = 0; break;
        case 3: result = 1; break;
        default: result = 0; break;
    }
    
    /* Minimal copy */
    int r = result;
    
    /* Compare to 1 */
    if (r != 1) {  /* Using != comparison */
        use(1100);
        return 11;
    } else {
        use(1200);
        return 12;
    }
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
    volatile int seed = g_seed;
    
    /* Call test functions with different inputs to exercise all paths */
    checksum += test_phi_simple_compare_zero(seed);
    checksum += test_phi_simple_compare_zero(-seed);
    
    checksum += test_phi_long_chain_compare_one(seed);
    checksum += test_phi_long_chain_compare_one(seed + 1);
    
    checksum += test_phi_three_way(-5);
    checksum += test_phi_three_way(0);
    checksum += test_phi_three_way(5);
    
    checksum += test_phi_with_loop(&seed);
    
    checksum += test_cascading_phis(seed, -seed);
    checksum += test_cascading_phis(-seed, seed);
    
    checksum += test_phi_from_switch(seed);
    checksum += test_phi_from_switch(seed + 1);
    checksum += test_phi_from_switch(seed + 2);
    checksum += test_phi_from_switch(seed + 3);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
