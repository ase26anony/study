/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime decisions */
volatile int g_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test_phi_basic_compare_zero(int input) {
    int val;
    if (input > 0) {
        val = 1;  /* Branch 1: constant 1 */
    } else {
        val = 0;  /* Branch 2: constant 0 */
    }
    
    /* Single copy assignment */
    int copy1 = val;
    
    /* Conditional branch comparing copy to 0 */
    if (copy1 == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Phi with longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test_phi_chain_compare_one(int input) {
    int val;
    if (input % 2 == 0) {
        val = 0;
    } else {
        val = 1;
    }
    
    /* Create a chain of copy assignments */
    int a = val;
    int b = a;
    int c = b;
    int d = c;
    
    /* Compare final copy to constant 1 */
    if (d == 1) {
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
    int val;
    if (input < 0) {
        val = 0;
    } else if (input == 0) {
        val = 1;
    } else {
        val = 0;  /* Different constant pattern */
    }
    
    /* Copy chain */
    int x = val;
    int y = x;
    
    /* Compare to 0 */
    if (y != 0) {  /* Note: != instead of == */
        use(500);
        return 5;
    } else {
        use(600);
        return 6;
    }
}

/* Test 4: Phi inside a loop with volatile control */
__attribute__((noinline, noipa))
int test_phi_in_loop(void) {
    volatile int limit = g_seed % 5 + 2;
    int sum = 0;
    
    for (int i = 0; i < limit; i++) {
        int val;
        if (i % 2 == 0) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy assignments */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional inside loop */
        if (tmp2 == 1) {
            sum += i * 2;
            use(sum);
        } else {
            sum += i;
            use(sum);
        }
    }
    return sum;
}

/* Test 5: Nested phi-copy patterns */
__attribute__((noinline, noipa))
int test_nested_phi_patterns(int input) {
    int result = 0;
    
    /* Outer conditional creates first phi */
    int outer_val;
    if (input > 100) {
        outer_val = 1;
    } else {
        outer_val = 0;
    }
    
    int copy_outer = outer_val;
    
    if (copy_outer == 1) {
        /* Inner conditional creates another phi */
        int inner_val;
        if (input % 3 == 0) {
            inner_val = 1;
        } else {
            inner_val = 0;
        }
        
        int copy_inner = inner_val;
        
        if (copy_inner == 0) {
            result = 7;
            use(700);
        } else {
            result = 8;
            use(800);
        }
    } else {
        result = 9;
        use(900);
    }
    
    return result;
}

/* Test 6: Phi with pointer copies (still SSA names) */
__attribute__((noinline, noipa))
int test_phi_with_indirect(int input) {
    int val;
    if (input % 4 == 0) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Create SSA copies through pointer operations */
    int* ptr1 = &val;
    int copy1 = *ptr1;
    int copy2 = copy1;
    
    /* Multiple comparisons */
    if (copy2 == 0) {
        use(1000);
        return 10;
    } else if (copy2 == 1) {
        use(1100);
        return 11;
    }
    
    return 12;
}

/* Test 7: Switch-based phi generation */
__attribute__((noinline, noipa))
int test_switch_phi(int input) {
    int val;
    
    switch (input % 3) {
        case 0:
            val = 0;
            break;
        case 1:
            val = 1;
            break;
        case 2:
            val = 0;
            break;
        default:
            val = 1;
    }
    
    /* Longer copy chain */
    int a = val;
    int b = a;
    int c = b;
    int d = c;
    int e = d;
    
    if (e == 1) {
        use(1200);
        return 13;
    }
    
    use(1300);
    return 14;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    g_seed = 42;
    
    /* Call each test function multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        checksum += test_phi_basic_compare_zero(i - 5);
        checksum += test_phi_chain_compare_one(i);
        checksum += test_phi_three_way(i - 2);
        checksum += test_phi_in_loop();
        checksum += test_nested_phi_patterns(i * 20);
        checksum += test_phi_with_indirect(i);
        checksum += test_switch_phi(i);
        
        /* Modify volatile to change execution paths */
        g_seed++;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Dummy implementation of external functions to satisfy linker */
void use(int x) {
    /* Empty - just to prevent optimization */
    (void)x;
}

void use_ptr(int* p) {
    /* Empty - just to prevent optimization */
    (void)p;
}
