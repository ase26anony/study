/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting save/restore instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile globals to prevent dead code elimination */
volatile int global_counter = 0;
volatile int global_sink = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_counter++; }
NOINLINE void func2(int x) { global_sink ^= x; }
NOINLINE void func3(void) { /* empty */ }
NOINLINE int func4(int a, int b) { return a ^ b; }

/* Test 1: Call at basic block end with many live values */
NOINLINE int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across call */
    int a = x * 3 + 1;
    int b = y << 2;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = (x + y) * (z - 1);
    int f = x | y | z;
    int g = ~(x & y);
    int h = (z << 4) | (x & 0xF);
    
    /* Function call with all values live */
    func1();
    
    /* Use all live values after call - forces spills/restores */
    return a + b - c + d * e - f + g ^ h;
}

/* Test 2: Call in switch case at basic block end */
NOINLINE int test_call_in_switch_case(int selector, int x, int y) {
    int result = 0;
    
    switch (selector & 3) {
        case 0: {
            /* Many live values in this case */
            int a = x + 1;
            int b = y * 2;
            int c = x ^ y;
            int d = (x << 3) | (y & 7);
            int e = ~x + y;
            int f = x * y - 1;
            
            /* Call at end of basic block before break */
            func2(a);
            
            /* This makes the call the BB_END before insertion */
            result = a + b + c + d + e + f;
            break;
        }
        case 1: {
            int t1 = x * x;
            int t2 = y * y;
            func3();
            result = t1 - t2;
            break;
        }
        default:
            result = x + y;
    }
    
    return result;
}

/* Test 3: Complex loop with values live across call */
NOINLINE int test_call_between_complex_ops(int iterations, int seed) {
    int acc = seed;
    
    /* Unrolled loop creates many live values */
    for (int i = 0; i < iterations && i < 8; i++) {
        /* Compute many independent values */
        int v1 = acc + i;
        int v2 = acc * i;
        int v3 = acc ^ i;
        int v4 = acc << (i & 3);
        int v5 = ~acc + i;
        int v6 = (acc & 0xFFFF) * i;
        int v7 = acc | (i << 8);
        int v8 = acc - i * 2;
        
        /* Call with all values live */
        void (*volatile fp)(void) = func3;
        fp();  /* Volatile call prevents optimization */
        
        /* Use all values after call */
        acc = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        
        /* Another call in same basic block */
        if (i & 1) {
            func1();
        }
    }
    
    return acc;
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        int a = x * 2 + 1;
        int b = y * 3 - 2;
        int c = z << 1;
        int d = a ^ b ^ c;
        
        if (y < 0) {
            int e = ~a + b;
            int f = c * d;
            int g = (a & b) | c;
            
            /* Call at end of inner basic block */
            func2(e);
            
            /* This return makes the call BB_END */
            return e + f + g;
        } else {
            int h = a + b + c;
            int i = d * 2;
            int j = h ^ i;
            
            /* Another call at block end */
            func1();
            
            result = h + i + j;
        }
    } else {
        result = y + z;
    }
    
    return result;
}

/* Test 5: Multiple calls with overlapping live ranges */
NOINLINE int test_multiple_calls(int x, int y) {
    /* Phase 1: Compute initial values */
    int a = x + 1;
    int b = y * 2;
    int c = x ^ y;
    int d = (x << 4) | (y & 0xF);
    
    /* First call - some values must be saved */
    int r1 = func4(a, b);
    
    /* More computations between calls */
    int e = r1 + c;
    int f = d * 2;
    int g = ~e;
    int h = f ^ g;
    
    /* Second call - different values live */
    func2(e);
    
    /* Final computation using all values */
    return a + b - c + d + e - f + g ^ h + r1;
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use command line args or defaults for variability */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        total += test_call_at_bb_end(base + i, base - i, base * i);
    }
    
    /* Test 2 with switch cases */
    total += test_call_in_switch_case(base & 3, base + 1, base - 1);
    total += test_call_in_switch_case((base + 1) & 3, base * 2, base / 2);
    
    /* Test 3 with loop */
    total += test_call_between_complex_ops(4, base);
    
    /* Test 4 with nested conditionals */
    total += test_nested_conditionals(base, -base, base ^ 0x55);
    total += test_nested_conditionals(-base, base, base & 0xAA);
    
    /* Test 5 with multiple calls */
    total += test_multiple_calls(base, base + 0x1000);
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
