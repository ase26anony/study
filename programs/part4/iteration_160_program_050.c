/* test-caller-save.c - Program to trigger caller-save optimization edge cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_sink = 0;

/* Non-inline helper functions that will force caller-save decisions */
__attribute__((noinline, noipa)) void non_inline_func1(void) {
    global_counter++;
}

__attribute__((noinline, noipa)) void non_inline_func2(int x) {
    global_sink ^= x;
}

__attribute__((noinline, noipa)) void non_inline_func3(void) {
    /* Empty but non-inline */
}

/* Test 1: Function call at the end of a basic block before return */
__attribute__((noinline)) int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across the call */
    int live1 = a * 3 + 1;
    int live2 = b ^ 0xABCD;
    int live3 = c & 0xFF00FF;
    int live4 = d | 0x12345678;
    int live5 = e << 3;
    int live6 = f >> 2;
    int live7 = a + b + c;
    int live8 = d - e - f;
    
    /* Additional computations to increase register pressure */
    int tmp1 = live1 * live2;
    int tmp2 = live3 | live4;
    int tmp3 = live5 ^ live6;
    int tmp4 = live7 & live8;
    
    /* Complex condition that creates a basic block ending with the call */
    if ((a + b + c) > (d + e + f)) {
        /* All these values must be preserved across the call */
        int sum_before = live1 + live2 + live3 + live4 + 
                        live5 + live6 + live7 + live8 +
                        tmp1 + tmp2 + tmp3 + tmp4;
        
        /* This call is at the end of the basic block before return */
        non_inline_func1();
        
        /* The BB_END should be the call, and inserted save/restore 
           should become the new BB_END */
        return sum_before + live1 + live2;  /* Use live values after call */
    } else {
        /* Alternative path also with register pressure */
        int prod = live1 * live2 * live3;
        non_inline_func2(prod);
        return prod + live4 + live5;
    }
}

/* Test 2: Function call in a switch case that ends with break */
__attribute__((noinline)) int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x * 2 + 1;
    int val2 = y ^ z;
    int val3 = (x & y) | z;
    int val4 = x + y + z;
    int val5 = x - y - z;
    int val6 = (x << 3) | (y << 2) | (z << 1);
    int val7 = ~(x ^ y ^ z);
    int val8 = (x * y) + (y * z) + (z * x);
    
    /* Switch creates multiple basic blocks */
    switch (x & 0x3) {
        case 0:
            /* Multiple computations before call */
            int tmp = val1 + val2 + val3;
            non_inline_func3();
            /* Call is before break, making it potentially BB_END */
            result = tmp + val4;
            break;
            
        case 1:
            /* Different computation pattern */
            result = val5 * val6;
            non_inline_func1();
            /* Use values after call */
            result += val7;
            break;
            
        case 2:
            /* Even more register pressure */
            int sum = val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8;
            for (int i = 0; i < 3; i++) {
                sum += i;
            }
            non_inline_func2(sum);
            result = sum ^ val1;
            break;
            
        default:
            result = val8;
            non_inline_func3();
            result += 1;
            break;
    }
    
    /* Use all values one more time to ensure liveness across calls */
    return result + (val1 & 0xFF) + (val2 & 0xFF) + (val3 & 0xFF);
}

/* Test 3: Complex loop with values live across call */
__attribute__((noinline)) int test_call_between_complex_ops(int seed) {
    /* Array-like computations without actual arrays to force register use */
    int acc1 = seed, acc2 = seed * 2, acc3 = seed * 3;
    int acc4 = seed * 4, acc5 = seed * 5, acc6 = seed * 6;
    int acc7 = seed * 7, acc8 = seed * 8, acc9 = seed * 9;
    int acc10 = seed * 10;
    
    /* Loop creates many live values */
    for (int i = 0; i < 5; i++) {
        acc1 = (acc1 ^ i) + 1;
        acc2 = (acc2 | i) * 3;
        acc3 = (acc3 & i) - 2;
        acc4 = (acc4 + i) ^ 0x1234;
        acc5 = (acc5 - i) | 0xABCD;
        
        /* Intermediate values that must be preserved */
        int t1 = acc1 + acc2;
        int t2 = acc3 * acc4;
        int t3 = acc5 ^ acc6;
        int t4 = acc7 & acc8;
        int t5 = acc9 | acc10;
        
        /* Function call with many live values */
        if (i & 1) {
            non_inline_func1();
        } else {
            non_inline_func2(t1);
        }
        
        /* Use all values after call */
        acc6 = t1 + t2;
        acc7 = t3 ^ t4;
        acc8 = t5 + i;
        acc9 = acc1 * acc2;
        acc10 = acc3 | acc4;
    }
    
    /* Final computation using all accumulated values */
    int final_result = acc1 + acc2 + acc3 + acc4 + acc5 + 
                      acc6 + acc7 + acc8 + acc9 + acc10;
    
    /* One more call at the end */
    non_inline_func3();
    
    return final_result;
}

/* Test 4: Nested conditionals with calls at block ends */
__attribute__((noinline)) int test_nested_conditionals(int a, int b, int c) {
    int x1 = a * 3, x2 = b * 5, x3 = c * 7;
    int x4 = a ^ b, x5 = b ^ c, x6 = a ^ c;
    int x7 = a & b, x8 = b & c, x9 = a & c;
    int x10 = a | b, x11 = b | c, x12 = a | c;
    
    if (a > 0) {
        int y1 = x1 + x2, y2 = x3 + x4;
        if (b > 0) {
            int z1 = y1 * y2, z2 = x5 ^ x6;
            non_inline_func1();  /* Call in nested block */
            /* Use values after call */
            int r1 = z1 + z2 + x7;
            
            if (c > 0) {
                int w1 = r1 * 2, w2 = x8 | x9;
                non_inline_func2(w1);  /* Another call */
                return w1 + w2 + x10;
            } else {
                non_inline_func3();
                return r1 + x11;
            }
        } else {
            int z3 = x10 ^ x11, z4 = x12 & 0xFF;
            non_inline_func2(z3);
            return z3 + z4;
        }
    } else {
        int y3 = x7 | x8, y4 = x9 ^ x10;
        non_inline_func3();
        return y3 + y4 + x11 + x12;
    }
}

/* Test 5: Volatile function pointer to prevent optimization */
__attribute__((noinline)) int test_volatile_funcptr(int base) {
    /* Declare volatile function pointer */
    void (*volatile fp1)(void) = non_inline_func1;
    void (*volatile fp2)(int) = non_inline_func2;
    
    /* Many live values */
    int v[8];
    for (int i = 0; i < 8; i++) {
        v[i] = base * (i + 1) + i;
    }
    
    /* Use volatile function pointers */
    int sum1 = v[0] + v[1] + v[2];
    fp1();  /* Call through volatile pointer */
    int sum2 = v[3] + v[4] + v[5] + sum1;
    fp2(sum2);  /* Another call */
    int sum3 = v[6] + v[7] + sum2;
    
    /* More computations between calls */
    int tmp1 = sum3 ^ v[0];
    int tmp2 = tmp1 & v[1];
    int tmp3 = tmp2 | v[2];
    fp1();
    int tmp4 = tmp3 + v[3];
    int tmp5 = tmp4 * v[4];
    fp2(tmp5);
    
    return tmp5 + v[5] + v[6] + v[7];
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
        total += test_call_at_bb_end(i*2, i*3, i*4, i*5, i*6, i*7);
    }
    
    /* Run test 2 */
    for (int i = 0; i < 8; i++) {
        total += test_call_in_switch_case(i, i*2, i*3);
        total ^= test_call_in_switch_case(i+10, i+20, i+30);
    }
    
    /* Run test 3 */
    for (int i = 1; i <= 5; i++) {
        total += test_call_between_complex_ops(i);
        total -= test_call_between_complex_ops(i * 10);
    }
    
    /* Run test 4 */
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            for (int k = -2; k <= 2; k++) {
                total += test_nested_conditionals(i, j, k);
            }
        }
    }
    
    /* Run test 5 */
    for (int i = 0; i < 20; i++) {
        total ^= test_volatile_funcptr(i);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    printf("Global sink: %d\n", global_sink);
    
    return (total != 0) ? 0 : 1;
}
