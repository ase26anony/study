/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fdump-rtl-all caller-save-test.c -o caller-save-test
 */

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

__attribute__((noinline, noipa)) void non_inline_func3(void) {
    /* Another empty function */
    asm volatile("" : : : "memory");
}

/* Test 1: Function call at the end of a basic block before return
 * This should create a basic block where the call is BB_END
 */
__attribute__((noinline)) int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across the call */
    int live1 = a * 3 + 1;
    int live2 = b << 2;
    int live3 = c & 0xFF;
    int live4 = d ^ e;
    int live5 = f * 7;
    int live6 = a + b + c;
    int live7 = d - e + f;
    int live8 = (a * b) / (c + 1);
    
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = non_inline_func1;
    
    /* Complex condition to create basic block structure */
    if (a > b) {
        /* More computations to increase register pressure */
        live1 += live2 * 2;
        live3 = live4 | live5;
        live6 = live7 ^ live8;
        
        /* Call at the end of basic block before return */
        fp();
        
        /* This return makes the call the last instruction in its basic block */
        return live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
    } else {
        /* Alternative path without call */
        return live1 - live2;
    }
}

/* Test 2: Function call in a switch case that ends with break
 * Creates basic blocks where call is at end before break
 */
__attribute__((noinline)) int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x * 2;
    int val2 = y + 5;
    int val3 = z & 0x0F;
    int val4 = x ^ y ^ z;
    int val5 = (x << 3) | (y << 1);
    int val6 = z * 11;
    int val7 = x + y + z;
    int val8 = (x * y) - z;
    
    switch (x % 4) {
        case 0:
            /* Use values before call */
            val1 += val2;
            val3 *= val4;
            
            /* Call then break - call is at end of basic block */
            non_inline_func2(val1);
            
            /* Break creates new basic block */
            result = val1 + val3;
            break;
            
        case 1:
            /* Different computation path */
            val5 = val6 ^ val7;
            result = val5 + val8;
            break;
            
        case 2:
            /* Another path with call */
            val2 = val3 * val4;
            non_inline_func3();
            result = val2 - val5;
            break;
            
        default:
            /* No call here */
            result = val6 + val7 + val8;
            break;
    }
    
    /* Use all values to ensure they're live across calls */
    return result + val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8;
}

/* Test 3: Complex loop with many live values across a call
 * Maximizes register pressure
 */
__attribute__((noinline)) int test_call_between_complex_ops(int iterations) {
    int i;
    /* Many accumulator variables */
    int acc1 = 1, acc2 = 2, acc3 = 3, acc4 = 4, acc5 = 5;
    int acc6 = 6, acc7 = 7, acc8 = 8, acc9 = 9, acc10 = 10;
    
    /* Loop creates many live values */
    for (i = 0; i < iterations && i < 10; i++) {
        /* Independent computations on each accumulator */
        acc1 = acc1 * 3 + i;
        acc2 = acc2 << 1 | i;
        acc3 = acc3 ^ (i * 7);
        acc4 = acc4 + (i << 2);
        acc5 = acc5 - (i / 2);
        acc6 = acc6 & (~i);
        acc7 = acc7 | (i * 11);
        acc8 = acc8 ^ acc1;
        acc9 = acc9 * acc2;
        acc10 = acc10 + acc3;
        
        /* Periodic function call with many live values */
        if (i == 5) {
            /* All accumulators are live across this call */
            non_inline_func1();
            
            /* More computations after call */
            acc1 += acc10;
            acc2 -= acc9;
        }
    }
    
    /* Use all accumulators to ensure they're live */
    return acc1 + acc2 + acc3 + acc4 + acc5 + 
           acc6 + acc7 + acc8 + acc9 + acc10;
}

/* Test 4: Nested conditionals with calls at block ends */
__attribute__((noinline)) int test_nested_conditionals(int a, int b, int c) {
    /* Create many live values */
    int x1 = a * b;
    int x2 = b * c;
    int x3 = c * a;
    int x4 = a + b + c;
    int x5 = a ^ b ^ c;
    int x6 = (a << b) | c;
    int x7 = (b >> 1) ^ a;
    int x8 = c * 3 + 5;
    
    void (*volatile fp)(void) = non_inline_func3;
    
    if (a > 0) {
        x1 += x2;
        x3 *= x4;
        
        if (b > 0) {
            x5 = x6 ^ x7;
            x8 += x1;
            
            /* Call at end of inner conditional block */
            fp();
            
            /* Return makes this the end of basic block */
            return x1 + x3 + x5 + x8;
        } else {
            x2 = x3 - x4;
            non_inline_func2(x2);
            return x2 + x6;
        }
    } else {
        if (c > 0) {
            x7 = x8 * 2;
            non_inline_func1();
            return x7 - x5;
        }
    }
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int total = 0;
    int i;
    
    /* Use command line arguments or defaults for variability */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (i = 0; i < 3; i++) {
        total += test_call_at_bb_end(base + i, base + i + 1, base + i + 2,
                                    base + i + 3, base + i + 4, base + i + 5);
    }
    
    /* Run test 2 */
    total += test_call_in_switch_case(base, base * 2, base * 3);
    total += test_call_in_switch_case(base + 1, base + 2, base + 3);
    
    /* Run test 3 */
    total += test_call_between_complex_ops(8);
    total += test_call_between_complex_ops(6);
    
    /* Run test 4 */
    total += test_nested_conditionals(base, base + 10, base + 20);
    total += test_nested_conditionals(-base, base - 10, base - 20);
    
    /* Store to volatile to prevent optimization */
    global_sink = total;
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
