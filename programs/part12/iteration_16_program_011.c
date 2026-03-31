/* test_autofdo_phi_copy_cond.c
 * Generates GIMPLE patterns with phi nodes, copy chains, and conditional branches
 * to cover uncovered lines in GCC's auto-profile.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int) __attribute__((noinline));
void use(int x) {
    /* Empty implementation - linker will resolve */
}

/* Global checksum to ensure all code paths are executed */
static volatile int checksum = 0;

/* Test 1: Basic phi with copy chain, compare to 0 */
__attribute__((noinline, noipa))
void test_phi_copy_compare_zero(volatile int cond) {
    int x;
    
    /* Create phi node with constants 0 and 1 */
    if (cond > 0) {
        x = 1;  /* Constant 1 */
    } else {
        x = 0;  /* Constant 0 */
    }
    
    /* Create copy propagation chain */
    int a = x;      /* First copy */
    int b = a;      /* Second copy */
    int c = b;      /* Third copy */
    int d = c;      /* Fourth copy */
    
    /* Conditional branch comparing final copy to 0 */
    if (d == 0) {
        use(100);
        checksum += 100;
    } else {
        use(200);
        checksum += 200;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
void test_phi_long_chain_compare_one(volatile int cond) {
    int y;
    
    /* Different branch structure for phi */
    if (cond & 1) {
        y = 0;
    } else if (cond & 2) {
        y = 1;
    } else {
        y = 0;  /* Third incoming edge to phi */
    }
    
    /* Longer copy chain */
    int t1 = y;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    int t6 = t5;
    
    /* Compare to 1 */
    if (t6 == 1) {
        use(300);
        checksum += 300;
    } else {
        use(400);
        checksum += 400;
    }
}

/* Test 3: Phi in loop context */
__attribute__((noinline, noipa))
void test_phi_in_loop(volatile int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        int val;
        
        /* Phi node inside loop */
        if (i & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional branch inside loop */
        if (tmp2 == 0) {
            total += 5;
        } else {
            total += 10;
        }
    }
    
    checksum += total;
    use(total);
}

/* Test 4: Nested if-else creating phi with multiple constants */
__attribute__((noinline, noipa))
void test_nested_phi_copy(volatile int a, volatile int b) {
    int result;
    
    /* Complex branching for richer phi */
    if (a > 0) {
        if (b > 0) {
            result = 1;
        } else {
            result = 0;
        }
    } else {
        if (b > 0) {
            result = 0;
        } else {
            result = 1;
        }
    }
    
    /* Copy chain with multiple variables */
    int copy1 = result;
    int copy2 = copy1;
    
    /* Compare to 0 using != operator */
    if (copy2 != 0) {  /* Should become == 1 after simplification */
        use(500);
        checksum += 500;
    } else {
        use(600);
        checksum += 600;
    }
}

/* Test 5: Switch-based phi with copy propagation */
__attribute__((noinline, noipa))
void test_switch_phi(volatile int selector) {
    int flag;
    
    /* Switch creates multiple incoming edges to phi */
    switch (selector % 4) {
        case 0:
            flag = 0;
            break;
        case 1:
            flag = 1;
            break;
        case 2:
            flag = 0;
            break;
        default:
            flag = 1;
            break;
    }
    
    /* Minimal copy chain */
    int f = flag;
    
    /* Conditional on the copy */
    if (f == 1) {
        use(700);
        checksum += 700;
    } else {
        use(800);
        checksum += 800;
    }
}

/* Test 6: Do-while loop with phi and copy */
__attribute__((noinline, noipa))
void test_loop_phi_copy(volatile int limit) {
    int i = 0;
    int sum = 0;
    
    do {
        int condition;
        
        /* Phi node depending on loop counter */
        if (i % 3 == 0) {
            condition = 1;
        } else {
            condition = 0;
        }
        
        /* Copy and conditional */
        int c = condition;
        if (c == 1) {
            sum += 3;
        } else {
            sum += 1;
        }
        
        i++;
    } while (i < limit && i < 20);
    
    checksum += sum;
    use(sum);
}

/* Test 7: Multiple independent phi-copy-conditional sequences */
__attribute__((noinline, noipa))
void test_multiple_sequences(volatile int x) {
    /* First sequence */
    int a1;
    if (x > 10) {
        a1 = 1;
    } else {
        a1 = 0;
    }
    int b1 = a1;
    if (b1 == 0) {
        checksum += 1;
    }
    
    /* Second sequence */
    int a2;
    if (x < 5) {
        a2 = 0;
    } else {
        a2 = 1;
    }
    int b2 = a2;
    int c2 = b2;
    if (c2 == 1) {
        checksum += 2;
    }
    
    /* Third sequence */
    int a3;
    if (x == 7) {
        a3 = 1;
    } else {
        a3 = 0;
    }
    int b3 = a3;
    int c3 = b3;
    int d3 = c3;
    if (d3 == 0) {
        checksum += 4;
    }
    
    use(checksum);
}

/* Main function that drives all tests */
int main(void) {
    volatile int seed = 0;
    
    /* Initialize volatile seed from environment or time */
    seed = getenv("SEED") ? atoi(getenv("SEED")) : 12345;
    
    /* Run all test patterns with varying inputs */
    test_phi_copy_compare_zero(seed);
    test_phi_long_chain_compare_one(seed + 1);
    test_phi_in_loop(seed % 5 + 1);
    test_nested_phi_copy(seed, seed + 1);
    test_switch_phi(seed);
    test_loop_phi_copy(seed % 10 + 1);
    test_multiple_sequences(seed);
    
    /* Print checksum to ensure all code executed */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
