/* test-caller-save.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions to force actual calls */
__attribute__((noinline, noipa))
void non_inline_func1(void) {
    /* Empty function - just to force a call */
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa))
void non_inline_func2(int x) {
    /* Use argument to prevent optimization */
    global_sink = x;
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa))
int non_inline_func3(int a, int b) {
    /* Return something to create register pressure */
    return a ^ b;
}

/* Test 1: Call at basic block end before return */
__attribute__((noinline, noipa))
int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values across the call */
    int a = x * 3 + 1;
    int b = y / 2 - 5;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = (x + y) * (z - 1);
    int f = (x << 3) | (y >> 2);
    int g = (z * 7) % 13;
    int h = x + y + z + 1;
    
    /* Complex condition to create basic block structure */
    if (x > 0 && y < 100) {
        /* All these values must be live across the call */
        int i = a + b;
        int j = c * d;
        int k = e ^ f;
        
        /* Function call at the end of basic block before return */
        non_inline_func1();
        
        /* This return makes the call the last instruction in the BB */
        return i + j + k + g + h;
    } else {
        /* Alternative path without call */
        return a + b + c + d;
    }
}

/* Test 2: Call in switch case with break */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int selector, int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int v1 = x * 2;
    int v2 = y + 3;
    int v3 = z - 5;
    int v4 = x ^ y;
    int v5 = y | z;
    int v6 = z & x;
    int v7 = (x + y) << 1;
    int v8 = (y - z) * 2;
    int v9 = (z + x) / 2;
    int v10 = x * y * z;
    
    switch (selector & 0x3) {
        case 0:
            /* Use some values before call */
            result = v1 + v2 + v3;
            /* Call at end of case before break */
            non_inline_func2(result);
            /* Break creates end of basic block */
            break;
            
        case 1:
            /* Different computation */
            result = v4 ^ v5 ^ v6;
            non_inline_func2(result);
            break;
            
        case 2:
            /* More register pressure */
            result = v7 + v8 + v9 + v10;
            /* Use volatile function pointer to ensure call isn't optimized */
            void (*volatile fp)(int) = non_inline_func2;
            fp(result);
            break;
            
        default:
            result = v1 - v2 - v3;
            break;
    }
    
    /* Use all values after switch to keep them live */
    return result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test 3: Call between complex operations with loop */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int iterations, int x, int y, int z) {
    /* Array-like computations to create register pressure */
    int values[16];
    int sum = 0;
    
    /* Compute many values in a way that prevents optimization */
    for (int i = 0; i < 8 && i < iterations; i++) {
        values[i*2] = (x + i) * (y - i);
        values[i*2 + 1] = (z ^ i) + (x & i);
    }
    
    /* Force all computed values to be live across call */
    int live1 = values[0] + values[1];
    int live2 = values[2] * values[3];
    int live3 = values[4] ^ values[5];
    int live4 = values[6] | values[7];
    int live5 = values[8] - values[9];
    int live6 = values[10] & values[11];
    int live7 = values[12] + values[13];
    int live8 = values[14] * values[15];
    
    /* Call with many live registers */
    int call_result = non_inline_func3(live1, live2);
    
    /* Use all live values after call */
    sum = live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
    
    /* More computations to increase pressure */
    int extra1 = (live1 << 2) | (live2 >> 1);
    int extra2 = (live3 * 3) % 17;
    int extra3 = (live4 ^ live5) + (live6 & live7);
    
    /* Another call */
    non_inline_func2(call_result);
    
    return sum + extra1 + extra2 + extra3 + call_result;
}

/* Test 4: Nested condition with call at BB end */
__attribute__((noinline, noipa))
int test_nested_conditions(int a, int b, int c, int d) {
    int result = 0;
    
    /* Many intermediate values */
    int t1 = a * b;
    int t2 = c + d;
    int t3 = a ^ c;
    int t4 = b | d;
    int t5 = (a + c) * 2;
    int t6 = (b - d) / 2;
    int t7 = t1 ^ t2;
    int t8 = t3 | t4;
    
    if (a > b) {
        if (c > d) {
            /* Deep nesting creates interesting BB structure */
            int inner1 = t1 + t2;
            int inner2 = t3 * t4;
            int inner3 = t5 ^ t6;
            
            /* Call at end of inner basic block */
            non_inline_func1();
            
            /* Return makes this BB end with the call */
            return inner1 + inner2 + inner3;
        } else {
            int inner4 = t7 - t8;
            int inner5 = t5 + t6;
            
            non_inline_func2(inner4);
            
            /* Different computation path */
            result = inner4 * inner5;
        }
    } else {
        result = t7 + t8;
    }
    
    /* Use remaining values */
    return result + t1 + t2 + t3 + t4;
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use command line arguments or defaults to vary inputs */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        total += test_call_at_bb_end(base + i, base - i, base * i);
    }
    
    /* Run test 2 with different selectors */
    for (int i = 0; i < 4; i++) {
        total += test_call_in_switch_case(i, base + 1, base + 2, base + 3);
    }
    
    /* Run test 3 with different iteration counts */
    for (int i = 1; i <= 3; i++) {
        total += test_call_between_complex_ops(i * 2, base, base + 10, base + 20);
    }
    
    /* Run test 4 with varied inputs */
    total += test_nested_conditions(base, base + 1, base + 2, base + 3);
    total += test_nested_conditions(base + 4, base, base + 5, base + 1);
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
