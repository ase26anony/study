/* Test program for GCC AutoFDO coverage of phi-copy-conditional patterns */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force variable reads */
volatile int g_volatile_seed = 0;

/* Test 1: Basic phi with 0/1 constants, short copy chain, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_zero(int input) {
    int result;
    
    /* Create phi node with 0/1 constants */
    if (input > 0) {
        result = 1;  /* Branch 1: constant 1 */
    } else {
        result = 0;  /* Branch 2: constant 0 */
    }
    
    /* Copy propagation chain */
    int a = result;  /* First copy */
    int b = a;       /* Second copy */
    int c = b;       /* Third copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (c == 0) {
        use(100);  /* Taken when c == 0 */
        return 1;
    } else {
        use(200);  /* Taken when c == 1 */
        return 2;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_copy_compare_one(int input) {
    int val;
    
    /* Different branch structure for phi */
    if (input & 1) {
        val = 0;
    } else if (input & 2) {
        val = 1;
    } else {
        val = 0;  /* Third incoming edge to phi */
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

/* Test 3: Phi with volatile-based branching */
__attribute__((noinline, noipa))
int test_phi_volatile_branch(void) {
    int x;
    
    /* Use volatile to prevent constant propagation */
    volatile int v = g_volatile_seed;
    
    if (v > 100) {
        x = 1;
    } else {
        x = 0;
    }
    
    /* Copy chain */
    int y = x;
    int z = y;
    
    /* Compare to 0 with != operator */
    if (z != 0) {
        use(500);
        return 5;
    }
    
    use(600);
    return 6;
}

/* Test 4: Nested control flow with phi */
__attribute__((noinline, noipa))
int test_nested_phi_copy(int input) {
    int temp;
    
    if (input > 50) {
        if (input < 100) {
            temp = 1;
        } else {
            temp = 0;
        }
    } else {
        temp = 0;
    }
    
    /* Copy through multiple variables */
    int copy1 = temp;
    int copy2 = copy1;
    
    if (copy2 == 1) {
        use(700);
        return 7;
    }
    
    use(800);
    return 8;
}

/* Test 5: Loop with phi-copy-conditional pattern */
__attribute__((noinline, noipa))
int test_loop_phi_pattern(int iterations) {
    int sum = 0;
    volatile int v = g_volatile_seed;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int flag;
        
        /* Phi node inside loop */
        if (v + i > 5) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        /* Copy chain inside loop */
        int f1 = flag;
        int f2 = f1;
        
        /* Conditional using copied value */
        if (f2 == 0) {
            sum += i * 2;
            use(900 + i);
        } else {
            sum += i;
            use(1000 + i);
        }
    }
    
    return sum;
}

/* Test 6: Multiple phi nodes feeding into each other */
__attribute__((noinline, noipa))
int test_multi_phi_chain(int input) {
    int a, b;
    
    /* First phi */
    if (input % 3 == 0) {
        a = 1;
    } else {
        a = 0;
    }
    
    /* Second phi dependent on first */
    if (a == 1) {
        b = 1;
    } else {
        b = 0;
    }
    
    /* Copy chain from second phi */
    int c = b;
    int d = c;
    int e = d;
    
    if (e == 1) {
        use(1100);
        return 11;
    }
    
    use(1200);
    return 12;
}

/* Test 7: Switch-based phi with copy chain */
__attribute__((noinline, noipa))
int test_switch_phi(int input) {
    int value;
    
    switch (input % 4) {
        case 0: value = 1; break;
        case 1: value = 0; break;
        case 2: value = 1; break;
        default: value = 0; break;
    }
    
    /* Copy chain */
    int v1 = value;
    int v2 = v1;
    
    if (v2 == 0) {
        use(1300);
        return 13;
    }
    
    use(1400);
    return 14;
}

/* Dummy implementation of external functions to satisfy linker */
void use(int x) {
    /* Empty - just to prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(void* p) {
    /* Empty */
    (void)p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_volatile_seed = 42;
    
    /* Call all test functions with different inputs */
    checksum += test_phi_copy_compare_zero(g_volatile_seed);
    checksum += test_phi_copy_compare_one(g_volatile_seed);
    checksum += test_phi_volatile_branch();
    checksum += test_nested_phi_copy(g_volatile_seed);
    checksum += test_loop_phi_pattern(8);
    checksum += test_multi_phi_chain(g_volatile_seed);
    checksum += test_switch_phi(g_volatile_seed);
    
    /* Vary inputs for more coverage */
    checksum += test_phi_copy_compare_zero(0);
    checksum += test_phi_copy_compare_one(3);
    checksum += test_nested_phi_copy(75);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
