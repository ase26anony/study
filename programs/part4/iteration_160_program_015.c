/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets instruction insertion at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noclone))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (void (*)(void))(func); \
    fp(); \
} while(0)

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void sink_func1(void) { global_sink = 1; }
NOINLINE void sink_func2(void) { global_sink = 2; }
NOINLINE void sink_func3(void) { global_sink = 3; }
NOINLINE void sink_func4(void) { global_sink = 4; }

/* Test 1: Call at basic block end with many live values */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e) {
    /* Create many live values that must survive across call */
    int live1 = a * 3 + 1;
    int live2 = b << 2;
    int live3 = c ^ 0x55AA55AA;
    int live4 = d + e * 7;
    int live5 = a ^ b ^ c;
    int live6 = d - e;
    int live7 = (a + b) * (c - d);
    int live8 = e | 0xFF00;
    
    /* Additional computations to increase register pressure */
    int tmp1 = live1 + live2;
    int tmp2 = live3 * live4;
    int tmp3 = live5 & live6;
    int tmp4 = live7 ^ live8;
    
    /* Function call at what could be BB end */
    if (a > b) {
        /* More computations before call */
        tmp1 += live2 * 3;
        tmp2 -= live4 / 2;
        tmp3 |= live6;
        tmp4 &= live8;
        
        /* Call with many values live across it */
        sink_func1();
        
        /* This return makes the call the last instruction in BB */
        return tmp1 + tmp2 + tmp3 + tmp4 + live1 + live2 + live3 + live4;
    } else {
        /* Alternative path to ensure both branches are compiled */
        return a + b + c + d + e;
    }
}

/* Test 2: Call in switch case with live values */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x * 2;
    int val2 = y + 5;
    int val3 = z ^ 0x12345678;
    int val4 = x + y + z;
    int val5 = x * y - z;
    int val6 = (x << 3) | (y << 1);
    int val7 = z * 7 + 1;
    int val8 = x ^ y ^ z;
    
    switch (x & 0x3) {
        case 0:
            /* Use all values before call */
            val1 += val2;
            val3 -= val4;
            val5 *= val6;
            val7 &= val8;
            
            /* Call in middle of case */
            sink_func2();
            
            /* Use values after call */
            result = val1 + val3 + val5 + val7;
            break;
            
        case 1:
            /* Different computation path */
            result = val2 - val4 + val6 - val8;
            break;
            
        case 2:
            /* Another path with call at end */
            val1 = val3 * val5;
            val2 = val4 ^ val6;
            sink_func3();
            /* Call could be BB_END before break */
            result = val1 + val2;
            break;
            
        default:
            result = x + y + z;
    }
    
    return result;
}

/* Test 3: Complex loop with call and many live values */
NOINLINE int test_call_between_complex_ops(int n, int seed) {
    int i;
    int accum[8] = {0};  /* Many accumulator variables */
    
    /* Initialize accumulators with complex computations */
    accum[0] = seed * 3;
    accum[1] = seed + 0xABCD;
    accum[2] = seed ^ 0xDEADBEEF;
    accum[3] = seed << 4;
    accum[4] = ~seed;
    accum[5] = seed * seed;
    accum[6] = seed / 3;
    accum[7] = seed | 0xFF;
    
    /* Loop creates register pressure */
    for (i = 0; i < n; i++) {
        /* Update all accumulators - creates many live values */
        accum[0] += i * 3;
        accum[1] ^= i + 1;
        accum[2] *= (i | 1);  /* Avoid multiply by 0 */
        accum[3] -= i << 2;
        accum[4] &= i ^ 0xAA;
        accum[5] |= i * i;
        accum[6] += accum[7] * i;
        accum[7] = accum[0] ^ accum[1];
    }
    
    /* Non-inline call with all accumulators live */
    /* Use volatile function pointer to ensure call isn't optimized */
    VOLATILE_CALL(sink_func4);
    
    /* Use all accumulators after call */
    int result = 0;
    for (i = 0; i < 8; i++) {
        result += accum[i];
        result ^= accum[7 - i];
    }
    
    return result;
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditional_calls(int a, int b, int c) {
    int x = a * 2;
    int y = b + 3;
    int z = c ^ 0x55;
    int w = a + b + c;
    
    if (a > 0) {
        int tmp1 = x * y;
        int tmp2 = z + w;
        
        if (b > 0) {
            int tmp3 = tmp1 ^ tmp2;
            int tmp4 = x + z;
            
            /* Call with many live values */
            sink_func1();
            
            /* This could make call the BB_END before return */
            return tmp3 + tmp4 + x + y;
        } else {
            int tmp3 = tmp1 & tmp2;
            sink_func2();
            return tmp3 + w;
        }
    } else {
        if (c > 0) {
            int tmp1 = y * z;
            sink_func3();
            /* Call at BB end before return */
            return tmp1 + a;
        } else {
            VOLATILE_CALL(sink_func4);
            return b + c;
        }
    }
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use command line args or defaults for variability */
    int base = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Run all tests multiple times with different inputs */
    total += test_call_at_bb_end(base, base+1, base+2, base+3, base+4);
    total += test_call_in_switch_case(base, base+10, base+20);
    total += test_call_between_complex_ops(5, base);
    total += test_nested_conditional_calls(base, base-1, base+100);
    
    /* Run again with different values */
    total += test_call_at_bb_end(base+100, base+101, base+102, base+103, base+104);
    total += test_call_in_switch_case(base+50, base+60, base+70);
    total += test_call_between_complex_ops(3, base+1000);
    total += test_nested_conditional_calls(base-100, base+50, base);
    
    /* Print result to prevent optimization */
    printf("Total result: %d\n", total);
    
    /* Also use global_sink to prevent dead code elimination */
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
