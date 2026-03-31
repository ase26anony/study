/* test-caller-save.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * caller-save.cc optimization pass. It creates scenarios where:
 * 1. Caller-save register spills are necessary due to many live values
 * 2. Function calls are at the end of basic blocks
 * 3. The inserted save/restore instructions become new BB_END
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to ensure actual function calls */
#define NOINLINE __attribute__((noinline, noclone))

/* Volatile function pointers to prevent optimization */
typedef void (*volatile_func_ptr)(void);

/* Global volatile variables to prevent dead code elimination */
volatile int global_sink = 0;
volatile int global_counter = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_counter++; }
NOINLINE void func2(void) { global_counter += 2; }
NOINLINE void func3(void) { global_counter += 3; }
NOINLINE void func4(void) { global_counter += 4; }
NOINLINE void func5(void) { global_counter += 5; }

/* Test 1: Call at the end of a basic block before return
 * This should create a basic block where the call is BB_END
 * and the inserted save/restore becomes the new BB_END
 */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across the call */
    int live1 = a * 2 + 1;
    int live2 = b * 3 - 2;
    int live3 = c ^ 0x55AA55AA;
    int live4 = d << 3;
    int live5 = e | 0xFF00FF00;
    int live6 = f & 0x00FF00FF;
    
    /* More intermediate computations to increase register pressure */
    int temp1 = live1 + live2;
    int temp2 = live3 - live4;
    int temp3 = live5 ^ live6;
    int temp4 = live1 * live3;
    int temp5 = live2 / (live4 ? live4 : 1);
    
    /* Use volatile function pointer to prevent optimization */
    volatile_func_ptr fp = func1;
    
    /* This call is at the end of a basic block before return */
    if (temp1 > temp2) {
        /* All these values must be live across the call */
        int sum = live1 + live2 + live3 + live4 + live5 + live6;
        int prod = temp1 * temp2 * temp3;
        int diff = temp4 - temp5;
        
        /* Call through volatile pointer - can't be optimized away */
        fp();
        
        /* Return uses all live values - they must be preserved */
        return sum + prod + diff;
    } else {
        /* Alternative path also with live values across call */
        int xor_result = live1 ^ live2 ^ live3;
        int shift_result = live4 << 2;
        
        fp = func2;
        fp();
        
        return xor_result + shift_result + temp3 + temp4 + temp5;
    }
}

/* Test 2: Call in a switch case that ends with break
 * Creates basic blocks ending with calls in switch arms
 */
NOINLINE int test_call_in_switch_case(int selector, 
                                      int x1, int x2, int x3,
                                      int x4, int x5, int x6) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x1 * x1 + 1;
    int val2 = x2 * x2 - 2;
    int val3 = x3 * x3 ^ 0x12345678;
    int val4 = x4 * x4 << 1;
    int val5 = x5 * x5 | 0x87654321;
    int val6 = x6 * x6 & 0xF0F0F0F0;
    
    /* Additional computations */
    int combo1 = val1 + val2 + val3;
    int combo2 = val4 * val5;
    int combo3 = val6 ^ (val1 & val2);
    
    switch (selector & 3) {
        case 0:
            /* Many live values across call, then break */
            result = val1 + val2;
            {
                volatile_func_ptr fp = func3;
                fp();  /* Call at end of basic block before break */
            }
            break;
            
        case 1:
            /* Different set of live values */
            result = val3 - val4;
            {
                int extra1 = combo1 * 2;
                int extra2 = combo2 / 2;
                volatile_func_ptr fp = func4;
                fp();  /* Call with many live values */
                result += extra1 + extra2;
            }
            break;
            
        case 2:
            /* Even more register pressure */
            result = val5 ^ val6;
            {
                int t1 = combo1 + combo2;
                int t2 = combo3 * 3;
                int t3 = val1 & val2 & val3;
                volatile_func_ptr fp = func5;
                fp();
                result = t1 + t2 + t3;
            }
            break;
            
        default:
            result = combo1 + combo2 + combo3;
            break;
    }
    
    /* Use all values to ensure they're live */
    global_sink = val1 + val2 + val3 + val4 + val5 + val6;
    return result + global_sink;
}

/* Test 3: Complex loop with values live across call at block end */
NOINLINE int test_call_between_complex_ops(int iterations, 
                                          int base1, int base2) {
    int accum1 = 0, accum2 = 0, accum3 = 0;
    int accum4 = 0, accum5 = 0, accum6 = 0;
    
    /* Create many accumulator variables that will be live */
    for (int i = 0; i < iterations; i++) {
        /* Each iteration creates values that could be in registers */
        int iter_val1 = base1 + i * 2;
        int iter_val2 = base2 - i * 3;
        int iter_val3 = (base1 ^ base2) + i;
        int iter_val4 = (base1 & base2) << i;
        int iter_val5 = (base1 | base2) >> (i & 3);
        int iter_val6 = (base1 * base2) % (i + 1 ? i + 1 : 1);
        
        /* Update accumulators - these must be kept live */
        accum1 += iter_val1;
        accum2 += iter_val2;
        accum3 ^= iter_val3;
        accum4 |= iter_val4;
        accum5 &= iter_val5;
        accum6 = accum6 * 2 + iter_val6;
        
        /* Periodic function call with many live values */
        if ((i % 5) == 0) {
            /* All accumulators are live across this call */
            volatile_func_ptr fp;
            if (i % 10 == 0) {
                fp = func1;
            } else {
                fp = func2;
            }
            
            /* This call happens in the middle of loop with many
             * live values - high register pressure */
            fp();
            
            /* More computations after call, using live values */
            accum1 = accum1 ^ iter_val6;
            accum2 = accum2 + iter_val5;
        }
    }
    
    /* Final call at what could be end of a basic block */
    if (accum1 > accum2) {
        volatile_func_ptr fp = func3;
        fp();  /* Call at potential BB_END */
        return accum1 + accum3 + accum5;
    } else {
        volatile_func_ptr fp = func4;
        fp();  /* Alternative call at BB_END */
        return accum2 + accum4 + accum6;
    }
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int a, int b, int c, 
                                      int d, int e, int f, int g) {
    /* Create many independent values */
    int v1 = a * b + c;
    int v2 = d * e - f;
    int v3 = g ^ 0xDEADBEEF;
    int v4 = (a & b) | (c & d);
    int v5 = (e ^ f) << (g & 3);
    int v6 = (a + b + c + d) % (e ? e : 1);
    int v7 = (f * g) / (a ? a : 1);
    int v8 = (b << c) ^ (d >> e);
    
    volatile_func_ptr fp;
    
    if (v1 > v2) {
        if (v3 > v4) {
            /* Deep nesting with many live values */
            int sum1 = v1 + v2 + v3 + v4;
            int prod1 = v5 * v6;
            fp = func1;
            fp();  /* Call at end of inner block */
            return sum1 + prod1;
        } else {
            int diff1 = v1 - v2 - v3;
            int xor1 = v4 ^ v5 ^ v6;
            fp = func2;
            fp();
            return diff1 + xor1;
        }
    } else {
        if (v5 < v6) {
            int combo1 = v7 + v8;
            int combo2 = v1 * v3;
            int combo3 = v2 / (v4 ? v4 : 1);
            fp = func3;
            fp();
            return combo1 + combo2 + combo3;
        } else {
            /* Many values live across this final call */
            int final1 = v1 + v3 + v5 + v7;
            int final2 = v2 + v4 + v6 + v8;
            int final3 = (v1 & v2) | (v3 & v4) | (v5 & v6);
            fp = func4;
            fp();  /* Call at very end of block before return */
            return final1 + final2 + final3;
        }
    }
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Starting caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
        total += test_call_at_bb_end(i*2, i*3, i*4, i*5, i*6, i*7);
    }
    
    /* Run test 2 with various selectors */
    for (int i = 0; i < 8; i++) {
        total += test_call_in_switch_case(i, 100+i, 200+i, 300+i, 
                                         400+i, 500+i, 600+i);
    }
    
    /* Run test 3 with different iteration counts */
    total += test_call_between_complex_ops(5, 42, 137);
    total += test_call_between_complex_ops(10, 123, 456);
    total += test_call_between_complex_ops(15, 789, 101112);
    
    /* Run test 4 with various inputs */
    for (int i = 0; i < 5; i++) {
        total += test_nested_conditionals(i, i*10, i*20, i*30, 
                                         i*40, i*50, i*60);
    }
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    /* Use result to prevent optimization */
    if (total > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
