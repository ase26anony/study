/* caller_save_test.c - Test program to trigger specific uncovered lines in GCC's caller-save.cc */

#include <stdio.h>
#include <stdlib.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will force caller-save decisions */
__attribute__((noinline, noipa)) void non_inline_func1(void) {
    /* Empty function - just a call target */
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa)) void non_inline_func2(int x) {
    /* Function with argument to prevent tail-call optimization */
    global_sink += x;
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa)) int non_inline_func3(int a, int b) {
    /* Function returning a value to create more register pressure */
    return a ^ b;
}

/* Test 1: Function call at the end of a basic block before return */
__attribute__((noinline, noipa)) int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across the call */
    int a = x * 2 + 1;
    int b = y / 3 - 5;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = x + y + z;
    int f = x * y - z;
    int g = (x << 3) | (y >> 2);
    int h = ~(x & y & z);
    
    /* All these values are now in registers and must be preserved */
    
    if (x > 0) {
        /* This creates a basic block ending with the call */
        int temp = a + b + c;
        non_inline_func1();  /* Call at BB end */
        /* After call, use all live values - forces save/restore */
        return temp + d + e + f + g + h;
    } else {
        /* Different path to ensure both branches are compiled */
        int temp = a - b - c;
        non_inline_func2(d);
        return temp - d - e - f - g - h;
    }
}

/* Test 2: Function call in a switch case */
__attribute__((noinline, noipa)) int test_call_in_switch_case(int selector, 
                                                              int v1, int v2, 
                                                              int v3, int v4) {
    /* Create register pressure */
    int r1 = v1 * v1;
    int r2 = v2 + v2;
    int r3 = v3 | 0x5555;
    int r4 = v4 & 0xAAAA;
    int r5 = v1 ^ v2;
    int r6 = v3 + v4;
    int r7 = v1 * v3;
    int r8 = v2 * v4;
    
    int result = 0;
    
    switch (selector & 3) {
        case 0:
            /* Basic block ending with call then break */
            r1 = r1 + r2;
            r3 = r3 - r4;
            non_inline_func1();
            break;
            
        case 1:
            r5 = r5 * r6;
            r7 = r7 / (r8 ? r8 : 1);
            non_inline_func2(r1);
            result = r5 + r7;
            break;
            
        case 2:
            /* Call with return value */
            result = non_inline_func3(r1, r2);
            result += r3 + r4;
            break;
            
        default:
            /* Multiple calls in sequence */
            non_inline_func1();
            non_inline_func2(r5);
            result = r6 + r7 + r8;
            break;
    }
    
    /* Use all values after switch to keep them live */
    return result + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

/* Test 3: Complex operations around call site */
__attribute__((noinline, noipa)) int test_call_between_complex_ops(int iterations, 
                                                                   int seed) {
    int i;
    int live_values[8];
    
    /* Compute many values in a loop to increase register pressure */
    for (i = 0; i < 8; i++) {
        live_values[i] = seed * (i + 1);
        live_values[i] ^= (live_values[i] << 3);
        live_values[i] += i * 17;
    }
    
    /* Additional computations to create more live ranges */
    int a = live_values[0] + live_values[1];
    int b = live_values[2] - live_values[3];
    int c = live_values[4] * live_values[5];
    int d = live_values[6] & live_values[7];
    int e = a ^ b;
    int f = c | d;
    int g = (a + c) * (b + d);
    int h = ~(e & f);
    
    /* Volatile function pointer to prevent inlining */
    void (*volatile fp)(void) = non_inline_func1;
    
    /* Call through volatile pointer - forces caller-save */
    fp();
    
    /* Use all computed values after the call */
    int sum = 0;
    for (i = 0; i < 8; i++) {
        sum += live_values[i];
    }
    
    return sum + a + b + c + d + e + f + g + h;
}

/* Test 4: Nested conditionals with calls at block ends */
__attribute__((noinline, noipa)) int test_nested_conditionals(int x, int y, int z) {
    int v1 = x * 3;
    int v2 = y + 7;
    int v3 = z ^ 0x99;
    int v4 = x & y & z;
    int v5 = v1 + v2;
    int v6 = v3 - v4;
    int v7 = v5 * v6;
    int v8 = ~v7;
    
    if (x > y) {
        if (y > z) {
            /* Inner basic block ending with call */
            int temp = v1 + v2 + v3;
            non_inline_func2(temp);
            /* This could make the call the BB_END */
            return temp + v4 + v5;
        } else {
            v6 = v6 * 2;
            non_inline_func1();
        }
        v7 = v7 / 2;
    } else {
        v8 = v8 | 0xFF;
        non_inline_func1();
        /* Call at end of else block */
    }
    
    /* Force all values to be live across some calls */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    int i;
    int result = 0;
    
    /* Use command line arguments or defaults for variability */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing caller-save insertion scenarios...\n");
    
    /* Run multiple iterations with different inputs */
    for (i = 0; i < 10; i++) {
        int val = base + i;
        
        /* Test 1: Call at BB end */
        result += test_call_at_bb_end(val, val + 1, val + 2);
        
        /* Test 2: Call in switch case */
        result += test_call_in_switch_case(val & 3, val, val + 10, 
                                          val + 20, val + 30);
        
        /* Test 3: Complex ops around call */
        result += test_call_between_complex_ops(5, val);
        
        /* Test 4: Nested conditionals */
        result += test_nested_conditionals(val, val * 2, val / 2);
    }
    
    printf("Final result: %d\n", result);
    
    /* Store to volatile to prevent optimization */
    global_sink = result;
    
    return result != 0 ? 0 : 1;
}
