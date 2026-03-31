/* test-caller-save.c
 * This program is designed to trigger specific uncovered lines in GCC's
 * caller-save.cc optimization pass, particularly the code that updates
 * BB_END when inserting save/restore instructions at the end of a basic block.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Non-inline functions that will be called */
__attribute__((noinline, noipa))
void non_inline_func1(void) {
    global_counter++;
}

__attribute__((noinline, noipa))
void non_inline_func2(int x) {
    global_counter += x;
}

__attribute__((noinline, noipa))
void non_inline_func3(void) {
    /* Empty function, just to force a call */
}

/* Test 1: Call at the end of a basic block before return */
__attribute__((noinline, noipa))
int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must be preserved across the call */
    int live1 = a * 2 + 1;
    int live2 = b * 3 - 2;
    int live3 = c & 0xFF;
    int live4 = d | 0x55;
    int live5 = e ^ f;
    int live6 = a + b + c;
    int live7 = d - e + f;
    int live8 = (a * b) >> 2;
    
    /* Use volatile function pointer to ensure call isn't optimized away */
    void (*volatile fp)(void) = non_inline_func1;
    
    /* Complex condition to create basic block structure */
    if (a > b) {
        /* More computations to increase register pressure */
        live1 += c * d;
        live2 -= e * f;
        live3 |= live4;
        live5 ^= live6;
        
        /* Function call with many live values across it */
        fp();
        
        /* This return makes the call the last instruction in the basic block */
        return live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
    } else {
        /* Alternative path without call */
        return a + b + c + d + e + f;
    }
}

/* Test 2: Call in a switch case that ends with break */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x * 3;
    int val2 = y * 5;
    int val3 = z * 7;
    int val4 = x + y + z;
    int val5 = x ^ y ^ z;
    int val6 = (x << 3) | (y << 2) | (z << 1);
    int val7 = ~(x & y & z);
    int val8 = x * y - z;
    
    switch (x % 4) {
        case 0:
            /* Use all live values before call */
            val1 += val2;
            val3 -= val4;
            val5 |= val6;
            val7 &= val8;
            
            /* Function call with many live values */
            non_inline_func2(val1);
            
            /* Break creates end of basic block after call */
            result = val1 + val3 + val5 + val7;
            break;
            
        case 1:
            result = val2 + val4 + val6 + val8;
            break;
            
        case 2:
            /* Another call site */
            val1 = val2 * val3;
            non_inline_func3();
            result = val1 + val4;
            break;
            
        default:
            result = x + y + z;
            break;
    }
    
    return result;
}

/* Test 3: Call between complex operations with loop-generated values */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int seed) {
    int i;
    int live_values[8];
    
    /* Generate many live values in a loop */
    for (i = 0; i < 8; i++) {
        live_values[i] = seed * (i + 1);
        live_values[i] ^= (seed << i);
        live_values[i] += i * 3;
    }
    
    /* Additional computations to increase register pressure */
    int sum1 = live_values[0] + live_values[1];
    int sum2 = live_values[2] + live_values[3];
    int sum3 = live_values[4] + live_values[5];
    int sum4 = live_values[6] + live_values[7];
    
    int prod1 = live_values[0] * live_values[1];
    int prod2 = live_values[2] * live_values[3];
    int prod3 = live_values[4] * live_values[5];
    int prod4 = live_values[6] * live_values[7];
    
    /* Use volatile function pointer */
    void (*volatile fp)(void) = non_inline_func3;
    
    /* Function call with many live values */
    fp();
    
    /* Use all values after call to ensure they remain live */
    int final_result = sum1 + sum2 + sum3 + sum4;
    final_result += prod1 + prod2 + prod3 + prod4;
    
    /* Mix in array values again */
    for (i = 0; i < 8; i++) {
        final_result ^= live_values[i];
    }
    
    return final_result;
}

/* Test 4: Multiple calls in different basic blocks */
__attribute__((noinline, noipa))
int test_multiple_calls(int a, int b) {
    int result = 0;
    
    /* First basic block with live values */
    int x1 = a * 2;
    int x2 = b * 3;
    int x3 = a + b;
    int x4 = a - b;
    
    if (a > 0) {
        int y1 = x1 * x2;
        int y2 = x3 * x4;
        
        /* Call at potential end of basic block */
        non_inline_func1();
        
        /* Complex condition to create another basic block */
        if (b > 0) {
            int z1 = y1 + y2;
            int z2 = x1 + x2;
            
            /* Another call */
            non_inline_func2(z1);
            
            result = z1 + z2;
        } else {
            result = y1 - y2;
        }
    } else {
        int y3 = x1 + x2 + x3 + x4;
        
        /* Call in else branch */
        non_inline_func3();
        
        result = y3 * 2;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    int i;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run each test multiple times with different inputs */
    for (i = 0; i < 100; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
        total += test_call_in_switch_case(i, i*2, i*3);
        total += test_call_between_complex_ops(i);
        total += test_multiple_calls(i, i*2);
    }
    
    /* Store result to volatile global to prevent optimization */
    global_result = total;
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return (total > 0) ? 0 : 1;
}
