/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) int external_func(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

/* Function pointer with unknown target */
int (*volatile func_ptr)(int) = external_func;

/* Stress function with complex control flow */
__attribute__((noinline)) 
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    long long v3 = seed * 3LL;
    float v4 = seed * 4.0f;
    double v5 = seed * 5.0;
    short v6 = seed * 6;
    char v7 = seed * 7;
    int v8 = seed * 8;
    long long v9 = seed * 9LL;
    float v10 = seed * 10.0f;
    double v11 = seed * 11.0;
    int v12 = seed * 12;
    int v13 = seed * 13;
    int v14 = seed * 14;
    int v15 = seed * 15;
    int v16 = seed * 16;
    
    /* Key intermediate result with complex computation */
    int key_result = v1 + v2 + (int)v3 + (int)v4 + (int)v5 + v6 + v7;
    
    /* Volatile control variables */
    volatile int ctrl1 = 1;
    volatile int ctrl2 = 0;
    volatile int ctrl3 = 1;
    
    /* Label addresses for complex control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = labels[seed % 5];
    
    /* Complex nested conditional structure */
    if (ctrl1) {
        if (ctrl2) {
            for (int i = 0; i < 3; i++) {
                switch (i) {
                    case 0:
                        key_result += v8;
                        break;
                    case 1:
                        /* Deeply nested conditional use of key_result */
                        if (ctrl3) {
                            /* Inline assembly that clobbers many registers */
                            __asm__ volatile (
                                "# Complex assembly block\n"
                                "add %0, %0, #1\n"
                                :
                                : "r" (key_result)
                                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                                  "memory", "cc"
                            );
                            
                            /* Call non-inlineable function */
                            key_result = func_ptr(key_result);
                        }
                        break;
                    case 2:
                        key_result -= v9;
                        break;
                }
            }
        } else {
            /* Another path with different computations */
            key_result *= 2;
            goto *volatile_label_ptr;
        }
    }
    
label1:
    /* Use key_result with different data types */
    v4 = (float)key_result;
    key_result += (int)v4;
    
    if (seed & 1) {
        goto label3;
    }
    
label2:
    /* More mixed-type computations */
    v5 = (double)key_result * 1.5;
    key_result += (int)v5;
    
    /* Another conditional block */
    if (ctrl1 && !ctrl2) {
        /* Use key_result in another inline assembly */
        __asm__ volatile (
            "# Another assembly use\n"
            "sub %0, %0, #10\n"
            : "+r" (key_result)
            :
            : "r0", "r1", "r2", "memory"
        );
    }
    
label3:
    /* Complex loop with conditional register use */
    for (int j = 0; j < 4; j++) {
        volatile int loop_ctrl = j;
        if (loop_ctrl == 2) {
            /* Conditional use inside loop */
            key_result = key_result * key_result - v10;
            
            /* More inline assembly with clobber */
            __asm__ volatile (
                "# Loop assembly\n"
                "mov %0, %0, lsr #2\n"
                : "+r" (key_result)
                :
                : "r0", "r1", "r2", "r3", "cc"
            );
        } else {
            key_result += v11;
        }
    }
    
label4:
    /* Final computations using all variables */
    int final_result = key_result;
    final_result += v12 + v13 + v14 + v15 + v16;
    final_result += (int)v3 + (int)v4 + (int)v5;
    final_result += v6 * v7;
    
    /* One more conditional path */
    if (ctrl3) {
        /* Use with different mode (64-bit) */
        long long ll_tmp = (long long)final_result * v9;
        final_result += (int)(ll_tmp >> 32);
        
        /* Mixed float/double operations */
        double d_tmp = (double)final_result / v5;
        float f_tmp = (float)d_tmp * v4;
        final_result += (int)f_tmp;
    }
    
label5:
    return final_result;
}

/* Additional complexity with recursion */
__attribute__((noinline))
int recursive_helper(int n, int acc) {
    if (n <= 0) return acc;
    
    volatile int recurse_ctrl = n & 3;
    switch (recurse_ctrl) {
        case 0:
            acc = stress_function(acc);
            break;
        case 1:
            acc += recursive_helper(n - 1, acc);
            break;
        case 2:
            acc *= 2;
            break;
        case 3:
            acc = func_ptr(acc);
            break;
    }
    
    return recursive_helper(n - 1, acc);
}

int main() {
    int total = 0;
    
    /* Create varying control flow patterns */
    for (int i = 0; i < 100; i++) {
        volatile int iter = i;
        
        if (iter & 1) {
            total += stress_function(i);
        } else {
            total += recursive_helper(i % 5, i);
        }
        
        /* Modify function pointer occasionally */
        if (iter % 7 == 0) {
            func_ptr = &external_func;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int output = total;
    return output % 256;
}
