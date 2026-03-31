/* caller_save_test.c - Test program to trigger specific uncovered lines in GCC's caller-save.cc */

#include <stdio.h>
#include <stdlib.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will force caller-save decisions */
void __attribute__((noinline,noipa)) non_inline_func1(void) {
    /* Empty function - just a call target */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline,noipa)) non_inline_func2(int x) {
    /* Use argument to prevent optimization */
    global_sink += x;
    asm volatile("" : : : "memory");
}

int __attribute__((noinline,noipa)) non_inline_func3(int a, int b) {
    /* Return something to create register pressure */
    return a ^ b;
}

/* Test 1: Call at basic block end with many live values */
int __attribute__((noipa)) test_call_at_bb_end(int x, int y, int z) {
    /* Create many independent live values */
    int a = x + 1;
    int b = y * 2;
    int c = z & 0xFF;
    int d = x ^ y;
    int e = y | z;
    int f = z - x;
    int g = x * y;
    int h = y + z;
    int i = z * 3;
    int j = x << 2;
    
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = non_inline_func1;
    
    /* Complex condition to create basic block structure */
    if (x > y) {
        /* All values are live across this call */
        fp();
        
        /* This return makes the call the last instruction in the basic block */
        return a + b + c + d + e + f + g + h + i + j;
    } else {
        /* Alternative path without call */
        int k = a * b;
        int l = c + d;
        return k + l + e + f;
    }
}

/* Test 2: Call in switch case with live values */
int __attribute__((noipa)) test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int v1 = x + y;
    int v2 = y * z;
    int v3 = x ^ z;
    int v4 = y << 1;
    int v5 = z >> 1;
    int v6 = x * 3;
    int v7 = y + 5;
    int v8 = z - x;
    int v9 = x & y;
    int v10 = y | z;
    
    switch (x % 4) {
        case 0:
            /* Use values before call */
            result = v1 + v2;
            non_inline_func2(v3);
            /* Call is at end of basic block before break */
            break;
            
        case 1:
            result = v3 * v4;
            /* Another call site */
            non_inline_func2(v5);
            break;
            
        case 2:
            /* More computations with live values */
            result = v6 + v7 + v8;
            non_inline_func2(v9);
            break;
            
        default:
            /* Use all live values in complex expression */
            result = v1 + v2 - v3 + v4 * v5 - v6 / 2 + v7 & v8 | v9 ^ v10;
            non_inline_func2(result);
            break;
    }
    
    /* Use all values after switch to extend liveness */
    return result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test 3: Call between complex operations with loop-generated values */
int __attribute__((noipa)) test_call_between_complex_ops(int x, int y, int z) {
    int values[10];
    
    /* Generate many values in a loop */
    for (int i = 0; i < 10; i++) {
        values[i] = x * i + y * (i + 1) + z * (i + 2);
    }
    
    /* More independent computations */
    int a = x * y + z;
    int b = y * z - x;
    int c = z * x ^ y;
    int d = (x + y) & z;
    int e = (y - z) | x;
    int f = (z + x) ^ y;
    int g = x * 7 + y * 3;
    int h = y * 5 - z * 2;
    int i = z * 11 ^ x;
    int j = x + y + z;
    
    /* Force register pressure by using all values before call */
    int sum1 = a + b + c + d + e;
    int sum2 = f + g + h + i + j;
    
    /* Call with many live values */
    int call_result = non_inline_func3(sum1, sum2);
    
    /* Use all values after call in complex computation */
    int final = call_result;
    for (int k = 0; k < 10; k++) {
        final += values[k] * (k + 1);
    }
    final += a * b - c / 2 + d & e | f ^ g - h + i * j;
    
    return final;
}

/* Test 4: Nested calls with live values */
int __attribute__((noipa)) test_nested_calls_live_values(int x, int y, int z) {
    /* Create a chain of computations */
    int live1 = x + 1;
    int live2 = y * 2;
    int live3 = z & 0xFF;
    int live4 = x ^ y ^ z;
    int live5 = (x << 3) | (y << 2) | (z << 1);
    int live6 = x * y * z;
    int live7 = y + z + x;
    int live8 = z - x + y;
    int live9 = (x & y) | (y & z) | (z & x);
    int live10 = x * 3 + y * 5 + z * 7;
    
    /* First call with many live values */
    non_inline_func2(live1);
    
    /* Intermediate computation that uses live values */
    int mid = live2 + live3 - live4;
    
    /* Second call - previous values still live */
    non_inline_func2(mid);
    
    /* More computations */
    int temp = live5 * live6 / 2;
    
    /* Third call in conditional block */
    if (temp > 100) {
        non_inline_func2(live7);
        /* Call at end of basic block before return */
        return live8 + live9 + live10 + temp;
    } else {
        int alt = live8 ^ live9;
        non_inline_func2(alt);
        return live10 * 2 + mid;
    }
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use command line arguments or defaults for variability */
    int x = argc > 1 ? atoi(argv[1]) : 12345;
    int y = argc > 2 ? atoi(argv[2]) : 67890;
    int z = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        total += test_call_at_bb_end(x + i, y - i, z * (i + 1));
        total += test_call_in_switch_case(y + i, z - i, x * (i + 2));
        total += test_call_between_complex_ops(z + i, x - i, y * (i + 3));
        total += test_nested_calls_live_values(x * (i + 1), y / (i + 1), z + i * 100);
    }
    
    /* Store result to volatile to prevent optimization */
    global_sink = total;
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
