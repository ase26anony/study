/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -c caller-save-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will force caller-save decisions */
__attribute__((noinline, noipa))
void non_inline_func1(void) {
    /* Empty function - just to force a call */
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa))
void non_inline_func2(int x) {
    /* Use argument to prevent optimization */
    global_sink += x;
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa))
int non_inline_func3(int a, int b) {
    /* Return something to create register pressure */
    return a ^ b;
}

/* Test 1: Call at basic block end with many live values */
__attribute__((noinline, noipa))
int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across call */
    int a = x + 1;
    int b = y * 2;
    int c = z & 0xFF;
    int d = x ^ y;
    int e = y | z;
    int f = z - x;
    int g = x * y;
    int h = y + z;
    
    /* Call at what should be BB_END */
    if (x > 0) {
        /* All these values are live across the call */
        non_inline_func1();
        /* This return makes the call the last instruction in BB */
        return a + b + c + d + e + f + g + h;
    } else {
        /* Alternative path to ensure control flow complexity */
        int i = x << 2;
        int j = y >> 1;
        non_inline_func2(i);
        return i + j;
    }
}

/* Test 2: Call in switch case with register pressure */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int val, int x, int y, int z) {
    int result = 0;
    
    switch (val & 3) {
        case 0: {
            /* Many live values before call */
            int a = x * 3;
            int b = y + 7;
            int c = z ^ 0x55;
            int d = x & y;
            int e = z | x;
            
            non_inline_func1();
            
            /* Use all live values after call */
            result = a + b - c + d * e;
            break;  /* Call is at BB_END before break */
        }
        case 1: {
            int f = x + y;
            int g = y * z;
            non_inline_func2(f);
            result = f - g;
            break;
        }
        case 2: {
            /* Even more register pressure */
            int h = x << 1;
            int i = y >> 2;
            int j = z & 0xF0;
            int k = x ^ y ^ z;
            int l = x + y + z;
            int m = x * z;
            
            non_inline_func1();
            
            result = h + i + j + k + l + m;
            break;
        }
        default:
            result = x + y + z;
    }
    
    return result;
}

/* Test 3: Call between complex operations with loop-generated values */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int seed, int iterations) {
    /* Create many independent values in a loop */
    int vals[8];
    
    for (int i = 0; i < 8; i++) {
        vals[i] = seed + i * 3;
        /* Do some computation to increase register pressure */
        vals[i] = (vals[i] * 7) ^ (seed << i);
    }
    
    /* Now create additional live values */
    int a = vals[0] + vals[1];
    int b = vals[2] * vals[3];
    int c = vals[4] ^ vals[5];
    int d = vals[6] & vals[7];
    int e = a ^ b;
    int f = c | d;
    int g = a + c;
    int h = b - d;
    
    /* Critical call with all values live */
    non_inline_func1();
    
    /* Use all values in complex computation */
    int result = 0;
    for (int i = 0; i < iterations; i++) {
        result += vals[i & 7] + a - b + c * d + e ^ f + g - h;
    }
    
    return result;
}

/* Test 4: Multiple calls in same function with overlapping live ranges */
__attribute__((noinline, noipa))
int test_multiple_calls(int x, int y, int z) {
    /* First set of live values */
    int a1 = x + 1;
    int b1 = y * 2;
    int c1 = z & 0xFF;
    
    /* First call - values a1,b1,c1 must be saved */
    int tmp = non_inline_func3(a1, b1);
    
    /* New values that overlap with old ones */
    int a2 = tmp + c1;
    int b2 = x ^ y;
    int c2 = y | z;
    int d2 = z - x;
    int e2 = x * y;
    
    /* Second call with different BB structure */
    if (tmp > 0) {
        non_inline_func2(a2);
        /* Call at BB_END before return */
        return a2 + b2 + c2 + d2 + e2;
    } else {
        int f2 = b2 << 2;
        int g2 = c2 >> 1;
        non_inline_func1();
        /* Another BB_END call scenario */
        return f2 * g2;
    }
}

/* Test 5: Volatile function pointer to prevent optimization */
__attribute__((noinline, noipa))
int test_volatile_call(int x, int y, int z) {
    /* Use volatile function pointer */
    void (*volatile fp)(void) = non_inline_func1;
    
    /* Many live values */
    int a = (x * 3) + (y * 5) - (z * 7);
    int b = (x ^ y) | (y ^ z) | (z ^ x);
    int c = (x & 0xF) << 4;
    int d = (y & 0xF0) >> 4;
    int e = z * z;
    int f = x + y + z;
    int g = x - y + z;
    int h = y - z + x;
    
    /* Volatile call - compiler can't analyze it */
    fp();
    
    /* Complex use of all values ensures they stay live */
    if (a > b) {
        return c + d + e + f + g + h;
    } else {
        return (c & d) | (e & f) ^ (g & h);
    }
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use command line args or defaults for variability */
    int x = argc > 1 ? atoi(argv[1]) : 12345;
    int y = argc > 2 ? atoi(argv[2]) : 67890;
    int z = argc > 3 ? atoi(argv[3]) : 54321;
    int iter = argc > 4 ? atoi(argv[4]) : 100;
    
    /* Run all tests, accumulating results */
    result += test_call_at_bb_end(x, y, z);
    result += test_call_in_switch_case(x, y, z, x);
    result += test_call_between_complex_ops(x, iter);
    result += test_multiple_calls(x, y, z);
    result += test_volatile_call(x, y, z);
    
    /* Store to volatile to prevent optimization */
    global_sink = result;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
