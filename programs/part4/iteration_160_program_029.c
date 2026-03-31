/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will force caller-save decisions */
__attribute__((noinline, noipa))
void non_inline_func1(void) {
    /* Empty function - just a call target */
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa))
void non_inline_func2(int x) {
    /* Function with argument to prevent tail-call optimization */
    global_sink += x;
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa))
int non_inline_func3(int a, int b) {
    /* Function that returns a value */
    asm volatile("" : : : "memory");
    return a ^ b;
}

/* Test 1: Call at basic block end with many live values */
__attribute__((noinline, noipa))
int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int live1 = a * 3 + 1;
    int live2 = b << 2;
    int live3 = c & 0xFF;
    int live4 = d | 0x1234;
    int live5 = e ^ f;
    int live6 = a + b + c;
    int live7 = d - e + f;
    int live8 = (a * b) / (c + 1);
    
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = non_inline_func1;
    
    /* Complex condition that creates basic block ending with call */
    if (live1 > live2) {
        /* All these values must be preserved across the call */
        int temp1 = live3 + live4;
        int temp2 = live5 * live6;
        int temp3 = live7 ^ live8;
        
        /* Call at the end of basic block before return */
        fp();
        
        /* This return makes the call the BB_END before insertion */
        return temp1 + temp2 + temp3;
    } else {
        /* Alternative path to ensure both branches are compiled */
        int temp4 = live1 | live2;
        int temp5 = live3 & live4;
        fp();
        return temp4 - temp5;
    }
}

/* Test 2: Call in switch case with register pressure */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int selector, int x1, int x2, int x3, int x4) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x1 * 2 + 1;
    int val2 = x2 << 3;
    int val3 = x3 & 0xABCD;
    int val4 = x4 | 0x1234;
    int val5 = x1 ^ x2 ^ x3;
    int val6 = x4 * 3 - x1;
    int val7 = (x2 + x3) * (x4 - x1);
    int val8 = x1 | x2 | x3 | x4;
    
    switch (selector & 3) {
        case 0:
            /* Many live values, then call, then break */
            {
                int t1 = val1 + val2;
                int t2 = val3 * val4;
                int t3 = val5 ^ val6;
                non_inline_func2(t1);
                /* Call is at end of basic block before break */
                result = t1 + t2 + t3;
            }
            break;
            
        case 1:
            {
                int t4 = val7 - val8;
                int t5 = val1 & val2;
                non_inline_func1();
                result = t4 | t5;
            }
            break;
            
        case 2:
            /* No call in this case for contrast */
            result = val3 + val4 + val5;
            break;
            
        default:
            {
                int t6 = val6 * val7;
                int t7 = val8 / (val1 + 1);
                void (*volatile fp2)(void) = non_inline_func1;
                fp2();
                result = t6 - t7;
            }
            break;
    }
    
    /* Use all values to keep them live */
    return result + (val1 & 0xFF) + (val2 >> 4);
}

/* Test 3: Call between complex operations with loop-generated values */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int seed, int iterations) {
    /* Array-like computations without actual arrays to force registers */
    int acc1 = seed;
    int acc2 = seed * 2;
    int acc3 = seed ^ 0x5555;
    int acc4 = seed | 0xAAAA;
    int acc5 = 0;
    int acc6 = 0;
    int acc7 = 0;
    int acc8 = 0;
    
    /* Generate many live values in a loop */
    for (int i = 0; i < iterations && i < 8; i++) {
        acc1 = acc1 * 3 + i;
        acc2 = acc2 ^ (acc1 << i);
        acc3 = acc3 + (acc2 >> 2);
        acc4 = acc4 | (acc3 & 0xFF);
        
        if (i == 3) {
            acc5 = acc1 + acc2;
            acc6 = acc3 * acc4;
        }
        
        if (i == 5) {
            acc7 = acc2 - acc3;
            acc8 = acc4 ^ acc1;
        }
    }
    
    /* All 8 accumulators are now live across the call */
    int sum_before = acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
    
    /* Force a function call that clobbers caller-saved registers */
    int ret_val = non_inline_func3(sum_before, acc1);
    
    /* Use all accumulators after the call */
    int sum_after = acc1 - acc2 + acc3 - acc4 + acc5 - acc6 + acc7 - acc8;
    
    /* Complex final computation using all live values */
    return ret_val + (sum_before & 0xFFFF) + (sum_after ^ 0xAAAA);
}

/* Test 4: Multiple consecutive calls with overlapping live ranges */
__attribute__((noinline, noipa))
int test_multiple_calls(int a, int b, int c, int d) {
    /* Create a web of dependent computations */
    int chain1 = a + b;
    int chain2 = chain1 * c;
    int chain3 = chain2 ^ d;
    int chain4 = chain3 | a;
    int chain5 = chain4 - b;
    int chain6 = chain5 & c;
    int chain7 = chain6 + d;
    int chain8 = chain7 ^ chain1;
    
    /* First call with some values live */
    non_inline_func2(chain1 + chain2);
    
    /* More computations between calls */
    int mid1 = chain3 * chain4;
    int mid2 = chain5 ^ chain6;
    
    /* Second call with different live set */
    non_inline_func1();
    
    /* Final computations using values from before first call */
    int final1 = chain7 + chain8 + mid1;
    int final2 = chain2 - chain4 + mid2;
    
    /* Third call at potential basic block end */
    if (final1 > final2) {
        int temp = chain1 + chain3 + chain5 + chain7;
        non_inline_func2(temp);
        return final1 + temp;  /* Call is BB_END before return */
    } else {
        non_inline_func1();
        return final2 - chain8;
    }
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use command line arguments or defaults to vary inputs */
    int base = argc > 1 ? atoi(argv[1]) : 12345;
    
    printf("Running caller-save insertion tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        result ^= test_call_at_bb_end(base + i, 
                                     base - i, 
                                     base * i, 
                                     base / (i + 1),
                                     base ^ i,
                                     base | i);
    }
    
    /* Run test 2 with different selectors */
    result += test_call_in_switch_case(base & 3, 
                                      base + 1, 
                                      base + 2, 
                                      base + 3, 
                                      base + 4);
    result += test_call_in_switch_case((base + 1) & 3, 
                                      base - 1, 
                                      base - 2, 
                                      base - 3, 
                                      base - 4);
    
    /* Run test 3 with different iteration counts */
    result ^= test_call_between_complex_ops(base, 4);
    result ^= test_call_between_complex_ops(base ^ 0x1234, 8);
    
    /* Run test 4 */
    result += test_multiple_calls(base, base + 11, base + 22, base + 33);
    
    /* Store to volatile global to prevent optimization */
    global_sink = result;
    
    printf("Final result: %d\n", result);
    printf("Global sink: %d\n", global_sink);
    
    return result != 0 ? 0 : 1;
}
