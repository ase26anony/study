/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Non-inline functions that will force caller-save decisions */
__attribute__((noinline, noipa))
void non_inline_func1(void) {
    /* Empty function - just a call site */
    global_counter++;
}

__attribute__((noinline, noipa))
void non_inline_func2(int x) {
    /* Function with argument to prevent tail-call optimization */
    global_counter += x;
}

__attribute__((noinline, noipa))
void non_inline_func3(void) {
    /* Another empty function */
    asm volatile("" : : : "memory");
}

/* Test 1: Call at basic block end with many live values */
__attribute__((noinline, noipa))
int test_call_at_bb_end(int a, int b, int c, int d, int e) {
    /* Create many live values that must survive across call */
    int live1 = a * 3 + 1;
    int live2 = b ^ 0x55AA55AA;
    int live3 = c << 3;
    int live4 = d | 0x0F0F0F0F;
    int live5 = e * 7 - 13;
    int live6 = (a + b) & 0xFF;
    int live7 = (c ^ d) | e;
    int live8 = a * b - c * d;
    
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = non_inline_func1;
    
    /* This call is at the end of a basic block before return */
    if (a > 0) {
        /* All these values must be saved across the call */
        fp();  /* This call should be BB_END before insertion */
        
        /* Return uses all live values - forces them to be preserved */
        return live1 + live2 - live3 + live4 * live5 - live6 + live7 ^ live8;
    } else {
        /* Different path to create control flow */
        return a + b + c;
    }
}

/* Test 2: Call in switch case with register pressure */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int selector, int x1, int x2, int x3, int x4) {
    int result = 0;
    
    /* Create many intermediate values */
    int val1 = x1 * 2 + 1;
    int val2 = x2 ^ x3;
    int val3 = x4 << 2;
    int val4 = x1 & x2 & x3;
    int val5 = x4 * 3 - 7;
    int val6 = (x1 + x2) | (x3 ^ x4);
    int val7 = x1 * x2 - x3 * x4;
    int val8 = (x1 << 3) | (x2 >> 1);
    
    switch (selector & 3) {
        case 0:
            /* Many live values across call in switch case */
            non_inline_func2(val1);
            /* Call is BB_END before break */
            result = val1 + val2;
            break;
            
        case 1:
            /* Different computation path */
            result = val3 - val4;
            break;
            
        case 2:
            /* Another path with call at end */
            non_inline_func3();
            /* Call is BB_END before break */
            result = val5 ^ val6;
            break;
            
        default:
            /* Use all values to keep them live */
            result = val7 + val8 + val1 + val2 + val3 + val4 + val5 + val6;
            non_inline_func1();
    }
    
    /* Final computation uses all values */
    return result + val1 - val2 + val3 * val4 - val5 ^ val6 | val7 & val8;
}

/* Test 3: Complex loop with call in middle, many live values */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int iterations, int seed) {
    int i;
    int accum[8] = {0};  /* Multiple accumulators */
    
    /* Initialize with complex computations */
    accum[0] = seed * 3;
    accum[1] = seed ^ 0x12345678;
    accum[2] = seed << 4;
    accum[3] = seed | 0x87654321;
    accum[4] = seed * 7 - 11;
    accum[5] = (seed + 1) & 0xFF;
    accum[6] = seed * seed;
    accum[7] = ~seed;
    
    /* Loop creates register pressure */
    for (i = 0; i < iterations && i < 10; i++) {
        /* Update all accumulators - creates many live values */
        accum[0] += i * 3;
        accum[1] ^= i;
        accum[2] <<= 1;
        accum[3] |= accum[0];
        accum[4] = accum[4] * 2 - i;
        accum[5] &= 0x7F;
        accum[6] += accum[1] * accum[2];
        accum[7] = ~accum[7];
        
        /* Call in middle of loop with many live values */
        if (i == iterations / 2) {
            /* This call has many registers live across it */
            non_inline_func2(accum[0]);
        }
    }
    
    /* Final reduction uses all accumulators */
    int result = 0;
    for (i = 0; i < 8; i++) {
        result += accum[i];
        result ^= accum[7 - i];
    }
    
    /* One more call at the end */
    non_inline_func3();
    
    return result;
}

/* Test 4: Nested conditionals with calls at block ends */
__attribute__((noinline, noipa))
int test_nested_conditionals(int a, int b, int c, int d) {
    /* Create many live values */
    int v1 = a + b;
    int v2 = c * d;
    int v3 = a ^ b ^ c;
    int v4 = (a << 4) | (b >> 4);
    int v5 = c * 3 - d * 2;
    int v6 = (a & b) | (c & d);
    int v7 = ~(a + c);
    int v8 = b * d + 17;
    
    /* Complex conditional structure */
    if (a > b) {
        if (c > d) {
            /* Call at end of inner block */
            non_inline_func1();
            /* This return makes the call BB_END */
            return v1 + v2 + v3;
        } else {
            v4 = v5 * v6;
            /* Another call at block end */
            non_inline_func2(v7);
        }
    } else {
        v8 = v1 * v2 - v3;
    }
    
    /* Use all values to keep them live */
    return v1 + v2 - v3 + v4 * v5 - v6 + v7 ^ v8;
}

/* Test 5: Multiple consecutive calls with overlapping live ranges */
__attribute__((noinline, noipa))
int test_multiple_calls(int a, int b, int c, int d, int e, int f) {
    /* Create overlapping live ranges */
    int x1 = a * 2;
    int x2 = b + c;
    non_inline_func1();  /* x1, x2 must be saved */
    
    int x3 = d ^ e;
    int x4 = x1 + x2;    /* x1, x2 still live */
    non_inline_func2(x3); /* x1, x2, x3, x4 must be saved */
    
    int x5 = f * 3;
    int x6 = x3 | x4;    /* x3, x4 still live */
    non_inline_func3();  /* x1-x6 must be saved */
    
    /* Use all values */
    return x1 + x2 - x3 + x4 * x5 - x6;
}

int main(void) {
    int total = 0;
    int i;
    
    printf("Starting caller-save coverage tests...\n");
    
    /* Run each test multiple times with different inputs */
    for (i = 0; i < 5; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4);
        total += test_call_in_switch_case(i, i*2, i*3, i*4, i*5);
        total += test_call_between_complex_ops(8, i*10);
        total += test_nested_conditionals(i, i+5, i+10, i+15);
        total += test_multiple_calls(i, i+1, i+2, i+3, i+4, i+5);
    }
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    /* Store to volatile to prevent dead code elimination */
    global_result = total;
    
    return (total > 0) ? 0 : 1;
}
