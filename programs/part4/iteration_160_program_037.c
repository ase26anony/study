/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions to force actual calls */
__attribute__((noinline, noipa)) void func1(void) {
    /* Empty function - just a call target */
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa)) void func2(int x) {
    /* Use argument to prevent optimization */
    global_sink += x;
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa)) void func3(void) {
    /* Another empty function */
    asm volatile("" : : : "memory");
}

/* Test 1: Call at basic block end with many live values */
__attribute__((noinline, noipa)) 
int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int live1 = a * 2 + 1;
    int live2 = b ^ c;
    int live3 = d - e;
    int live4 = f << 2;
    int live5 = a + b + c;
    int live6 = d * e * f;
    int live7 = (a & b) | (c & d);
    int live8 = e ^ f ^ a;
    
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = func1;
    
    /* Complex condition to create basic block boundary */
    if (a > b) {
        /* More computations to increase register pressure */
        live1 += live2 * 3;
        live3 -= live4 / 2;
        live5 = live6 ^ live7;
        live8 = live8 * 2 + 1;
        
        /* Function call at the end of basic block (before return) */
        fp();
        
        /* This return makes the call the BB_END before insertion */
        return live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
    } else {
        /* Alternative path without call */
        live1 -= live2;
        live3 += live4;
        return live1 * live3;
    }
}

/* Test 2: Call in switch case with live values */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x * 3;
    int val2 = y + 7;
    int val3 = z ^ 0xFF;
    int val4 = x + y + z;
    int val5 = x * y - z;
    int val6 = (x << 4) | (y << 2) | z;
    int val7 = val1 ^ val2 ^ val3;
    int val8 = val4 & val5 & val6;
    
    switch (x % 4) {
        case 0:
            /* Use values before call */
            val1 += val2;
            val3 -= val4;
            /* Function call in the middle of case */
            func2(val1);
            /* Break creates basic block end after call */
            break;
            
        case 1:
            val5 = val6 * 2;
            func1();
            /* Call at end of basic block before break */
            break;
            
        case 2:
            /* Multiple calls with live values */
            val7 = val8 + val1;
            func3();
            val2 = val3 * val4;
            break;
            
        default:
            val8 = val1 - val2;
            break;
    }
    
    /* Use all values after switch to keep them live across calls */
    result = val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8;
    return result;
}

/* Test 3: Complex loop with call and many live values */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int iterations, int seed) {
    int i;
    /* Array of values to create register pressure */
    int vals[12];
    int sum = 0;
    
    /* Initialize with computations */
    for (i = 0; i < 12; i++) {
        vals[i] = seed * i + (i * i);
    }
    
    /* Loop creates many live values */
    for (i = 0; i < iterations; i++) {
        /* Update all values - creates many simultaneous live values */
        vals[0] = vals[1] ^ vals[2];
        vals[1] = vals[3] + vals[4];
        vals[2] = vals[5] * vals[6];
        vals[3] = vals[7] - vals[8];
        vals[4] = vals[9] & vals[10];
        vals[5] = vals[11] | vals[0];
        vals[6] = vals[1] << 2;
        vals[7] = vals[2] >> 1;
        vals[8] = vals[3] * 3;
        vals[9] = vals[4] + 7;
        vals[10] = vals[5] ^ 0x55;
        vals[11] = vals[6] - vals[7];
        
        /* Function call with many values live across it */
        if (i % 3 == 0) {
            func1();
        }
        
        /* More computations after call */
        vals[0] += vals[11];
        vals[1] -= vals[10];
        vals[2] *= vals[9];
    }
    
    /* Use all values to keep them live */
    for (i = 0; i < 12; i++) {
        sum += vals[i];
    }
    
    return sum;
}

/* Test 4: Nested conditionals with calls at block ends */
__attribute__((noinline, noipa))
int test_nested_conditionals(int a, int b, int c) {
    int x1 = a + b;
    int x2 = b * c;
    int x3 = c ^ a;
    int x4 = a * 3 + b;
    int x5 = b << c;
    int x6 = c - a;
    int x7 = (a & b) | c;
    int x8 = x1 ^ x2 ^ x3;
    
    if (a > 0) {
        x1 += x2;
        if (b > 0) {
            x3 *= x4;
            /* Call at end of inner basic block */
            func2(x1);
            /* Return makes this BB_END */
            return x1 + x3;
        } else {
            x5 = x6 + x7;
            func1();
            /* Another BB_END candidate */
            return x5 + x8;
        }
    } else {
        x2 -= x3;
        if (c > 0) {
            x4 = x5 * x6;
            func3();
            return x4;
        }
    }
    
    /* Final computation using all values */
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
}

/* Main function to run all tests */
int main(void) {
    int total = 0;
    int i;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
    }
    
    /* Run test 2 */
    for (i = 0; i < 8; i++) {
        total += test_call_in_switch_case(i, i*2, i*3);
    }
    
    /* Run test 3 */
    total += test_call_between_complex_ops(5, 42);
    
    /* Run test 4 */
    for (i = 0; i < 6; i++) {
        total += test_nested_conditionals(i, i-1, i+1);
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
