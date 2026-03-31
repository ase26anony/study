/* test-caller-save.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * caller-save.cc optimization pass. It creates scenarios where:
 * 1. Multiple integer values must remain live across function calls
 * 2. Function calls are placed at the end of basic blocks
 * 3. Register pressure is high enough to force spill/restore decisions
 * 
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -o test-caller-save test-caller-save.c
 * For coverage analysis: add -fdump-rtl-caller-save -fdump-rtl-all
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will be called */
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

/* Test 1: Call at the end of a basic block before return
 * This should create a basic block where the call is BB_END
 */
__attribute__((noipa))
int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across the call */
    int a = x * 3 + 1;
    int b = y << 2;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = (x + y) * (z - 1);
    int f = ~(x | y | z);
    int g = (x << 3) | (y << 2) | (z << 1);
    int h = x * y - z;
    
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = non_inline_func1;
    
    /* Different control flow paths to create basic block boundaries */
    if (x > 0) {
        /* This branch ends with a call then return - call is at BB_END */
        int temp1 = a + b;
        int temp2 = c - d;
        fp();  /* Non-inline call with many live values */
        
        /* Use all live values after call */
        int result = temp1 + temp2 + e + f + g + h;
        return result;
    } else {
        /* Alternative path without call at BB_END */
        int temp1 = a * b;
        int temp2 = c / (d + 1);
        non_inline_func2(temp1);
        
        /* More operations to create different basic block structure */
        int result = temp2 + e - f + g * h;
        if (y > 0) {
            result += non_inline_func3(a, b);
        }
        return result;
    }
}

/* Test 2: Call in a switch case that ends with break
 * Creates basic blocks where cases end with calls
 */
__attribute__((noipa))
int test_call_in_switch_case(int selector, int x, int y, int z) {
    /* Create register pressure */
    int v1 = x + 1;
    int v2 = y * 2;
    int v3 = z & 0x0F;
    int v4 = x ^ y;
    int v5 = y ^ z;
    int v6 = z ^ x;
    int v7 = (x + y + z) * 3;
    int v8 = ~(x & y & z);
    
    int result = 0;
    
    switch (selector & 3) {
        case 0:
            /* This case ends with call then break - call is at BB_END */
            v1 = v1 * v2 + v3;
            v4 = v4 | v5 | v6;
            non_inline_func1();  /* Call at end of basic block */
            result = v1 + v4 + v7 + v8;
            break;
            
        case 1:
            /* Call not at BB_END in this case */
            v2 = v2 - v3;
            non_inline_func2(v1);
            v5 = v5 * v6;
            result = v2 + v5 + v7 - v8;
            break;
            
        case 2:
            /* Multiple calls in one case */
            v3 = v3 << 2;
            non_inline_func1();
            v6 = v6 >> 1;
            non_inline_func2(v3);
            result = v3 + v6 + v7 * v8;
            break;
            
        default:
            /* Call through volatile pointer */
            void (*volatile fp)(void) = non_inline_func1;
            v7 = v7 & 0xAA;
            fp();
            v8 = v8 | 0x55;
            result = v7 ^ v8;
            break;
    }
    
    /* Use all values one more time to ensure liveness */
    return result + v1 + v2 + v3 + v4 + v5 + v6;
}

/* Test 3: Complex loop creating many live values, then a call
 * Maximizes register pressure
 */
__attribute__((noipa))
int test_call_between_complex_ops(int iterations, int x, int y, int z) {
    /* Array of values to create many live ranges */
    int values[16];
    int i;
    
    /* Initialize with computations */
    for (i = 0; i < 16; i++) {
        values[i] = (x * i) + (y * (i + 1)) - (z * (i + 2));
    }
    
    /* More live variables */
    int a = x * y * z;
    int b = x + y + z;
    int c = x ^ y ^ z;
    int d = ~(x & y & z);
    int e = (x << 4) | (y << 2) | z;
    int f = (x > y) ? x - y : y - x;
    int g = (y > z) ? y * z : z * y;
    int h = (z > x) ? z / (x + 1) : x / (z + 1);
    
    /* Non-inline call with many live values */
    non_inline_func1();
    
    /* Use all values after call to ensure they're live across it */
    int sum = 0;
    for (i = 0; i < 16; i++) {
        sum += values[i];
    }
    
    /* Complex computation using all live variables */
    int result = sum + a - b + c * d + e / (f + 1) + g - h;
    
    /* Volatile store to prevent optimization */
    global_sink = result;
    
    return result;
}

/* Test 4: Nested conditionals with calls at block ends */
__attribute__((noipa))
int test_nested_conditionals(int cond1, int cond2, int x, int y, int z) {
    /* Create many live values */
    int live1 = x * 3 + 7;
    int live2 = y << 1;
    int live3 = z & 0x7F;
    int live4 = x ^ y;
    int live5 = y ^ z;
    int live6 = z ^ x;
    int live7 = (x + 1) * (y + 2) * (z + 3);
    int live8 = ~(live1 | live2 | live3);
    
    int result = 0;
    
    if (cond1) {
        if (cond2) {
            /* Inner block ending with call */
            live1 = live1 + live2;
            live3 = live3 - live4;
            non_inline_func1();  /* Call at BB_END of inner block */
            result = live1 + live3 + live5 + live6;
            /* No else here - fall through to return */
        } else {
            /* Different path */
            live2 = live2 * live3;
            live4 = live4 | live5;
            non_inline_func2(live2);
            live6 = live6 & live7;
            result = live2 - live4 + live6 + live8;
        }
        return result;
    } else {
        /* Outer else with call at end */
        live5 = live5 << 2;
        live7 = live7 >> 1;
        live8 = live8 ^ 0xFF;
        non_inline_func1();  /* Call at BB_END before return */
        return live5 + live7 + live8;
    }
}

/* Test 5: Multiple calls in sequence with overlapping live ranges */
__attribute__((noipa))
int test_multiple_calls(int x, int y, int z) {
    /* Phase 1: Compute initial values */
    int a1 = x + 1;
    int b1 = y * 2;
    int c1 = z & 0x0F;
    int d1 = x ^ y;
    
    /* First call - some values must survive */
    non_inline_func1();
    
    /* Phase 2: More computations using results from phase 1 */
    int a2 = a1 * 3;
    int b2 = b1 + c1;
    int c2 = d1 << 1;
    int d2 = a1 ^ b1 ^ c1;
    int e2 = (a1 + b1) * (c1 + 1);
    
    /* Second call - even more values must survive */
    non_inline_func2(a2);
    
    /* Phase 3: Final computations */
    int a3 = a2 + b2;
    int b3 = c2 - d2;
    int c3 = e2 & 0xFF;
    
    /* Third call at potential BB_END */
    if (x > y) {
        int temp = a3 * b3;
        non_inline_func1();  /* Call at BB_END of if block */
        return temp + c3;
    } else {
        non_inline_func2(b3);
        return a3 + b3 + c3;
    }
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int total = 0;
    int i;
    
    /* Use command line arguments or defaults to vary inputs */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run each test multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        int x = base + i;
        int y = base * 2 - i;
        int z = base / 2 + i * 3;
        
        total += test_call_at_bb_end(x, y, z);
        total += test_call_in_switch_case(i, x, y, z);
        total += test_call_between_complex_ops(5, x, y, z);
        total += test_nested_conditionals(i & 1, i & 2, x, y, z);
        total += test_multiple_calls(x, y, z);
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return (total > 0) ? 0 : 1;
}
