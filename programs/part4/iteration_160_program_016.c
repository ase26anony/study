/* test-caller-save.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets lines 905-913 which handle instruction insertion
 * at the end of basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noclone))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (void (*)(void))func; \
    fp(); \
} while(0)

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Call at the end of a basic block before return
 * This should create a basic block where the call is BB_END
 * and the inserted save/restore becomes the new BB_END
 */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across the call */
    int live1 = a * 2 + 1;
    int live2 = b ^ c;
    int live3 = d & 0xFF;
    int live4 = e << 2;
    int live5 = f * 3;
    int live6 = a + b + c;
    int live7 = d | e;
    int live8 = f - a;
    
    /* Complex condition to create control flow */
    if (a > b && c < d) {
        /* More computations to increase register pressure */
        live1 = live1 * live2 + 1;
        live2 = live2 ^ live3;
        live3 = live3 & live4;
        live4 = live4 << live5;
        live5 = live5 * live6;
        live6 = live6 + live7;
        live7 = live7 | live8;
        live8 = live8 - live1;
        
        /* Function call at the end of basic block before return */
        func1();
        
        /* This return makes the call the last instruction in the basic block */
        return live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
    } else {
        /* Alternative path with different computations */
        live1 = live1 + live8;
        live2 = live2 * live7;
        live3 = live3 ^ live6;
        live4 = live4 & live5;
        
        /* Another call at basic block end */
        func2();
        
        return live1 - live2 + live3 - live4;
    }
}

/* Test 2: Call in a switch case with break
 * Creates basic blocks that end with calls
 */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x * y;
    int val2 = y + z;
    int val3 = z ^ x;
    int val4 = x & y & z;
    int val5 = (x << 3) | (y << 2) | (z << 1);
    int val6 = ~val1;
    int val7 = val2 * 3;
    int val8 = val3 / 2;
    
    switch (x % 4) {
        case 0:
            /* Use all live values before call */
            val1 = val1 + val2;
            val2 = val2 * val3;
            val3 = val3 ^ val4;
            val4 = val4 | val5;
            val5 = val5 & val6;
            val6 = val6 + val7;
            val7 = val7 - val8;
            val8 = val8 * 2;
            
            /* Call then break - call is at end of basic block */
            func3();
            break;
            
        case 1:
            val1 = val1 - val8;
            val2 = val2 ^ val7;
            val3 = val3 & val6;
            func1();
            break;
            
        case 2:
            val4 = val4 | val5;
            val5 = val5 * val1;
            func2();
            break;
            
        default:
            val6 = val6 + val7;
            val7 = val7 - val8;
            func4();
            break;
    }
    
    /* Use all values after the switch to keep them live */
    result = val1 + val2 - val3 * val4 / (val5 + 1) + (val6 ^ val7) | val8;
    return result;
}

/* Test 3: Complex loop with call in middle
 * Creates high register pressure with values live across call
 */
NOINLINE int test_call_between_complex_ops(int iterations, int seed) {
    int i;
    /* Many accumulator variables */
    int acc1 = seed, acc2 = seed * 2, acc3 = seed + 1;
    int acc4 = seed ^ 0x55, acc5 = seed & 0xFF;
    int acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Pre-loop computations creating more live values */
    int tmp1 = acc1 * 3;
    int tmp2 = acc2 ^ acc3;
    int tmp3 = acc4 | acc5;
    int tmp4 = acc1 + acc2 + acc3;
    int tmp5 = acc4 - acc5;
    
    for (i = 0; i < iterations; i++) {
        /* Update accumulators in complex ways */
        acc1 = acc1 + tmp1 + i;
        acc2 = acc2 ^ tmp2 ^ i;
        acc3 = acc3 * (tmp3 & 0xF) + i;
        acc4 = acc4 | (tmp4 << (i & 3));
        acc5 = acc5 & (tmp5 >> (i & 3));
        acc6 = acc6 + acc1;
        acc7 = acc7 ^ acc2;
        acc8 = acc8 - acc3;
        
        /* Every 8 iterations, make a function call
         * with all these values live */
        if ((i & 7) == 7) {
            /* All accumulators and tmps are live here */
            VOLATILE_CALL(func1);
            
            /* More computations after call */
            tmp1 = tmp1 ^ acc4;
            tmp2 = tmp2 + acc5;
            tmp3 = tmp3 * acc6;
            tmp4 = tmp4 | acc7;
            tmp5 = tmp5 & acc8;
        }
    }
    
    /* Final computation using all values */
    int result = acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
    result = result ^ tmp1 ^ tmp2 ^ tmp3 ^ tmp4 ^ tmp5;
    return result;
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int a, int b, int c) {
    int x = a, y = b, z = c;
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    
    /* Outer conditional */
    if (a > 0) {
        /* First level computations */
        r1 = x * y + z;
        r2 = y ^ z ^ x;
        r3 = (x & y) | z;
        r4 = x << (y & 3);
        
        /* Inner conditional */
        if (b > 0) {
            /* More computations */
            int t1 = r1 + r2;
            int t2 = r3 * r4;
            int t3 = r1 ^ r2 ^ r3;
            int t4 = r4 & 0xFF;
            
            /* Call at end of inner block */
            func2();
            
            /* Use values after call */
            r1 = t1 + t2;
            r2 = t3 | t4;
        } else {
            int t5 = r1 - r2;
            int t6 = r3 ^ r4;
            
            /* Another call at block end */
            func3();
            
            r3 = t5 * t6;
            r4 = t5 + t6;
        }
        
        /* Call at end of outer block */
        func1();
    } else {
        /* Alternative path */
        r1 = x + y + z;
        r2 = x * y * z;
        
        func4();
    }
    
    return r1 + r2 + r3 + r4;
}

int main(void) {
    int total = 0;
    int i;
    
    printf("Testing caller-save edge cases...\n");
    
    /* Run multiple tests with different inputs */
    for (i = 0; i < 100; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
        total += test_call_in_switch_case(i, i*2, i*3);
        total += test_call_between_complex_ops(50, i);
        total += test_nested_conditionals(i, i-10, i+10);
    }
    
    printf("Total result: %d\n", total);
    printf("(This value varies based on inputs - main point is execution)\n");
    
    return 0;
}
