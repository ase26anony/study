/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline,noipa))
int test_phi_basic_compare_zero(int input) {
    int x;
    if (input > 0) {
        x = 1;  /* Branch 1: constant 1 */
    } else {
        x = 0;  /* Branch 2: constant 0 */
    }
    
    /* Copy chain */
    int a = x;      /* First copy */
    int b = a;      /* Second copy */
    
    /* Conditional comparing final copy to 0 */
    if (b == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline,noipa))
int test_phi_long_chain_compare_one(int input) {
    int y;
    if (input % 2 == 0) {
        y = 0;  /* Even: 0 */
    } else {
        y = 1;  /* Odd: 1 */
    }
    
    /* Longer copy propagation chain */
    int t1 = y;
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

/* Test 3: Phi with three incoming edges */
__attribute__((noinline,noipa))
int test_phi_three_way(int input) {
    int z;
    if (input < -10) {
        z = 0;
    } else if (input > 10) {
        z = 1;
    } else {
        z = 0;  /* Same constant from different edge */
    }
    
    int copy1 = z;
    int copy2 = copy1;
    
    if (copy2 != 0) {  /* Compare to 0 with != operator */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Nested control flow with phi */
__attribute__((noinline,noipa))
int test_phi_nested(int input) {
    int w = 0;
    
    /* Outer if creates phi at inner join point */
    if (input != 0) {
        if (input > 100) {
            w = 1;
        } else {
            w = 0;
        }
    } else {
        w = 1;
    }
    
    int chain1 = w;
    int chain2 = chain1;
    
    if (chain2 == 0) {
        use(700);
        return 7;
    }
    use(800);
    return 8;
}

/* Test 5: Phi in loop with copy chain */
__attribute__((noinline,noipa))
int test_phi_in_loop(int iterations) {
    int sum = 0;
    volatile int vol = g_seed;  /* Prevent loop unrolling */
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        if (vol & 1) {  /* Volatile prevents constant propagation */
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int c1 = val;
        int c2 = c1;
        
        if (c2 == 1) {
            sum += i * 2;
            use(sum);
        } else {
            sum += i;
            use(sum);
        }
        
        /* Modify volatile to vary path */
        vol = vol >> 1;
    }
    return sum;
}

/* Test 6: Multiple phi nodes feeding each other */
__attribute__((noinline,noipa))
int test_phi_cascade(int input) {
    int x, y;
    
    /* First phi */
    if (input > 50) {
        x = 1;
    } else {
        x = 0;
    }
    
    /* Second phi depends on first through copy */
    int tmp = x;
    if (tmp == 1) {
        y = 0;
    } else {
        y = 1;
    }
    
    /* Copy chain from second phi */
    int final1 = y;
    int final2 = final1;
    int final3 = final2;
    
    if (final3 == 0) {
        use(900);
        return 9;
    } else {
        use(1000);
        return 10;
    }
}

/* Test 7: Switch-based phi with constants */
__attribute__((noinline,noipa))
int test_phi_switch(int input) {
    int result;
    
    switch (input % 4) {
        case 0: result = 0; break;
        case 1: result = 1; break;
        case 2: result = 0; break;
        default: result = 1; break;
    }
    
    int r1 = result;
    int r2 = r1;
    
    if (r2 == 1) {
        use(1100);
        return 11;
    }
    use(1200);
    return 12;
}

/* Dummy use function to prevent optimization */
void use(int x) {
    /* Empty - just to create side effect */
    static volatile int sink;
    sink = x;
}

void use_ptr(int *p) {
    static volatile int* sink_ptr;
    sink_ptr = p;
}

int main(void) {
    int checksum = 0;
    volatile int seed = g_seed;
    
    /* Call all test functions with varying inputs */
    checksum += test_phi_basic_compare_zero(seed);
    checksum += test_phi_long_chain_compare_one(seed + 1);
    checksum += test_phi_three_way(seed - 5);
    checksum += test_phi_nested(seed + 10);
    checksum += test_phi_in_loop(5);
    checksum += test_phi_cascade(seed + 20);
    checksum += test_phi_switch(seed + 3);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs with different seeds to exercise all paths */
    for (int i = 0; i < 3; i++) {
        g_seed = i * 7;
        test_phi_basic_compare_zero(g_seed);
        test_phi_long_chain_compare_one(g_seed);
    }
    
    return 0;
}
