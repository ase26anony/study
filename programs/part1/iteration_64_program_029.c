#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier = 0;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 100; i++) {
        a += b ^ c;
        b = c * a + i;
        c = (a ^ b) + g_var1;
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" : : : "memory");
        
        d = a + b;
        e = b * c;
        f = c - a;
        g = d ^ e;
        h = f * g;
        
        /* Data-dependent exit condition */
        if (__builtin_expect_with_probability(h > 1000000, 0, 0.3)) {
            barrier = 1;
            break;
        }
        
        /* More dependent operations */
        a = (a + h) ^ barrier;
        b = (b - h) | barrier;
        c = (c * 2) + barrier;
    }
    
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int val) {
    int local1 = val, local2 = val * 2, local3 = val + 1;
    int local4 = 0, local5 = 0, local6 = 0;
    
    /* Complex arithmetic chain */
    local1 = (local1 ^ local2) + g_var2;
    local2 = local2 * local3 - depth;
    local3 = (local3 << 3) | depth;
    
    /* Memory operation with uncertain latency */
    volatile int* ptr = &g_var1;
    local4 = *ptr + depth;
    
    if (depth > 0) {
        /* Recursive call - scheduler may save/restore state around this */
        local5 = recursive_compute(depth - 1, local1);
        local6 = recursive_compute(depth - 1, local2);
    }
    
    /* Another scheduling barrier */
    asm volatile("" : : : "memory");
    
    return local1 + local2 + local3 + local4 + local5 + local6;
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int selector) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Large switch with different computation patterns in each case */
    switch (selector % 12) {
        case 0:
            v1 += v2 * v3;
            v4 = v5 ^ v6;
            v7 = v8 - v9;
            v10 = v11 | v12;
            break;
        case 1:
            v2 = v3 + v4;
            v5 = v6 * v7;
            v8 = v9 ^ v10;
            v11 = v12 - v13;
            break;
        case 2:
            v3 = v4 | v5;
            v6 = v7 + v8;
            v9 = v10 * v11;
            v12 = v13 ^ v14;
            break;
        case 3:
            v4 = v5 - v6;
            v7 = v8 | v9;
            v10 = v11 + v12;
            v13 = v14 * v15;
            break;
        case 4:
            v5 = v6 ^ v7;
            v8 = v9 - v10;
            v11 = v12 | v13;
            v14 = v15 + v16;
            break;
        case 5:
            v6 = v7 * v8;
            v9 = v10 ^ v11;
            v12 = v13 - v14;
            v15 = v16 | v17;
            break;
        case 6:
            v7 = v8 + v9;
            v10 = v11 * v12;
            v13 = v14 ^ v15;
            v16 = v17 - v18;
            break;
        case 7:
            v8 = v9 | v10;
            v11 = v12 + v13;
            v14 = v15 * v16;
            v17 = v18 ^ v19;
            break;
        case 8:
            v9 = v10 - v11;
            v12 = v13 | v14;
            v15 = v16 + v17;
            v18 = v19 * v20;
            break;
        case 9:
            v10 = v11 ^ v12;
            v13 = v14 - v15;
            v16 = v17 | v18;
            v19 = v20 + v1;
            break;
        case 10:
            v11 = v12 * v13;
            v14 = v15 ^ v16;
            v17 = v18 - v19;
            v20 = v1 | v2;
            break;
        case 11:
            v12 = v13 + v14;
            v15 = v16 * v17;
            v18 = v19 ^ v20;
            v1 = v2 - v3;
            break;
    }
    
    /* Merge point with complex data flow */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

__attribute__((noinline, optimize("O3")))
int loop_with_inner_function(int iterations) {
    int result = 0;
    
    /* Outer loop calling inner function - may create scheduling regions */
    for (int i = 0; i < iterations; i++) {
        int inner_result = 0;
        
        /* Inner loop with software pipelining potential */
        for (int j = 0; j < 8; j++) {
            inner_result += (i * j) ^ g_array[(i + j) & 255];
            
            /* Irregular control flow with goto */
            if (__builtin_expect_with_probability((inner_result & 0xFF) == 0, 0, 0.1)) {
                goto early_exit;
            }
            
            /* Do-while with break */
            int k = 0;
            do {
                if (k > 3) break;
                inner_result += k * g_var2;
                k++;
            } while (1);
        }
        
        early_exit:
        result += inner_result;
        
        /* Pointer aliasing to create uncertainty */
        int* alias1 = (int*)&g_var1;
        int* alias2 = (int*)&g_array[i & 255];
        *alias1 = (*alias1 + i) ^ *alias2;
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize array with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    checksum += compute_chain(42);
    
    /* Kernel 2: Recursive computation */
    checksum += recursive_compute(4, 73);
    
    /* Kernel 3: Complex switch statement */
    for (int i = 0; i < 50; i++) {
        checksum += switch_complex(i);
    }
    
    /* Kernel 4: Nested loops with inner function */
    checksum += loop_with_inner_function(100);
    
    /* Additional stress: mix of all patterns */
    for (int i = 0; i < 20; i++) {
        if (i & 1) {
            checksum += compute_chain(i * 7);
        } else {
            checksum += switch_complex(checksum);
        }
        
        /* Unpredictable branch */
        if (__builtin_expect_with_probability((checksum & 0xF) == 0, 0, 0.2)) {
            checksum += recursive_compute(2, i);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
