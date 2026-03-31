/* test_autofdo_phi_patterns.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(void*);

/* Global volatile to force runtime decisions */
volatile int g_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test1(volatile int cond) {
    int val;
    if (cond > 0) {
        val = 1;  /* Will become phi argument 1 */
    } else {
        val = 0;  /* Will become phi argument 0 */
    }
    
    /* Copy chain of length 1 */
    int a = val;
    
    /* Conditional comparing copy to 0 */
    if (a == 0) {
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy chain (3 copies), compare to 1 */
__attribute__((noinline, noipa))
int test2(volatile int cond) {
    int val;
    if (cond & 1) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Longer copy chain */
    int t1 = val;
    int t2 = t1;
    int t3 = t2;
    
    /* Compare to 1 (not 0) */
    if (t3 == 1) {
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi with three incoming edges (if-else if-else) */
__attribute__((noinline, noipa))
int test3(volatile int cond) {
    int val;
    if (cond > 10) {
        val = 1;
    } else if (cond < -10) {
        val = 0;
    } else {
        val = 1;  /* Same as first branch to test phi merging */
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

/* Test 4: Phi in loop context */
__attribute__((noinline, noipa))
int test4(volatile int iter_count) {
    int sum = 0;
    int limit = (iter_count & 3) + 2;  /* Limit 2-5 */
    
    for (int i = 0; i < limit; i++) {
        int val;
        volatile int inner_cond = g_seed + i;
        
        if (inner_cond & 1) {
            val = 1;
        } else {
            val = 0;
        }
        
        /* Copy chain inside loop */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Conditional inside loop */
        if (tmp2 == 0) {
            sum += i * 10;
        } else {
            sum += i * 20;
        }
    }
    return sum;
}

/* Test 5: Nested copy chains with different comparison types */
__attribute__((noinline, noipa))
int test5(volatile int cond1, volatile int cond2) {
    int val1, val2;
    
    /* First phi */
    if (cond1 > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    /* Second phi */
    if (cond2 > 0) {
        val2 = 1;
    } else {
        val2 = 0;
    }
    
    /* Interleaved copy chains */
    int a = val1;
    int b = val2;
    int c = a;
    int d = b;
    int e = c;
    
    /* Multiple conditionals */
    int result = 0;
    if (e == 0) {
        result += 10;
        use(700);
    }
    
    if (d == 1) {
        result += 20;
        use(800);
    }
    
    return result;
}

/* Test 6: Phi with boolean constants from function arguments */
__attribute__((noinline, noipa))
int test6(int flag1, int flag2) {
    int val;
    
    /* More complex condition to avoid simplification */
    if ((flag1 ^ flag2) & 1) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Minimal copy */
    int v = val;
    
    /* Compare to 1 */
    if (v == 1) {
        return 111;
    } else {
        return 222;
    }
}

/* Test 7: Switch-based phi pattern */
__attribute__((noinline, noipa))
int test7(volatile int code) {
    int val;
    
    switch (code & 3) {
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
            break;
    }
    
    /* Two-level copy */
    int x1 = val;
    int x2 = x1;
    
    if (x2 == 0) {
        use(900);
        return 33;
    } else {
        use(1000);
        return 44;
    }
}

/* Main driver that executes all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        volatile int input = g_seed + i * 17;
        
        checksum ^= test1(input);
        checksum ^= test2(input);
        checksum ^= test3(input);
        checksum ^= test4(input);
        checksum ^= test5(input, input + 1);
        checksum ^= test6(i, i + 1);
        checksum ^= test7(input);
        
        /* Modify global to affect subsequent iterations */
        g_seed += 5;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
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
