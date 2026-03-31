/* reload_test.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    volatile int y = x;
    return y ^ 0x55;
}

/* Complex structure with nested arrays */
struct nested {
    int data[4][8];
    volatile long offsets[3];
    union {
        float f;
        int i;
    } pun;
};

/* Global volatile to force memory operations */
volatile int global_index = 0;
volatile long global_base = 1000;

/* Test function with maximum register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
int test_reloads(int a1, int a2, int a3, int a4, int a5, 
                 int a6, int a7, int a8, int a9, int a10) {
    /* Declare many local variables to exhaust registers */
    register int r1 asm ("r12") = a1 + 1;
    register int r2 asm ("r13") = a2 + 2;
    int v1 = a3, v2 = a4, v3 = a5, v4 = a6, v5 = a7;
    int v6 = a8, v7 = a9, v8 = a10;
    long l1 = a1 * 3L, l2 = a2 * 5L, l3 = a3 * 7L;
    float f1 = a4 * 1.5f, f2 = a5 * 2.5f;
    double d1 = a6 * 3.14159;
    
    /* Complex multi-dimensional array with volatile index */
    volatile int idx1 = global_index % 4;
    volatile int idx2 = global_index % 8;
    struct nested ns[3];
    
    /* Initialize structure */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                ns[i].data[j][k] = i * 100 + j * 10 + k;
            }
        }
        ns[i].offsets[0] = global_base + i * 8;
        ns[i].offsets[1] = global_base + i * 16;
        ns[i].offsets[2] = global_base + i * 24;
        ns[i].pun.i = i * 0x12345678;
    }
    
    /* SECTION 1: Complex addressing modes requiring secondary reloads */
    /* Access with SIB-like addressing: base + index*scale + offset */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        /* Force complex address calculation */
        long base = ns[i].offsets[0];
        int index = idx1 * 2 + i;
        int scale = 4;
        
        /* This should trigger reloads for addressing */
        sum += ns[i].data[index % 4][idx2] * (base + index * scale);
    }
    
    /* SECTION 2: Inline assembly with register clobbering */
    /* Force many register reloads around asm */
    asm volatile (
        "/* Begin massive clobber */\n\t"
        "add %[v1], %[v2], %[v3]\n\t"
        "sub %[v4], %[v5], %[v6]\n\t"
        : [v1] "+r" (v1), [v2] "+r" (v2), [v3] "+r" (v3),
          [v4] "+r" (v4), [v5] "+r" (v5), [v6] "+r" (v6)
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r14", "r15", "memory", "cc"
    );
    
    /* SECTION 3: Mixed register class operations */
    /* Integer to float and back */
    ns[0].pun.f = f1 + f2;
    int int_from_float = ns[0].pun.i;
    
    /* Use atomic operations for additional reload complexity */
    _Atomic int atomic_var = 0;
    __atomic_store_n(&atomic_var, v1 + v2, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* SECTION 4: More complex addressing with inline assembly constraints */
    /* Force memory operand with complex address into asm */
    volatile int* volatile_ptr = &ns[idx1].data[idx2][0];
    
    int asm_result;
    asm volatile (
        "ldr %0, [%1, %2, lsl #2]\n\t"  /* Complex ARM addressing */
        : "=r" (asm_result)
        : "r" (volatile_ptr), "r" (idx1 * 2)
        : "memory"
    );
    
    /* SECTION 5: Long dependency chain with all variables */
    r1 = barrier(r1 + v1 + v2 + v3);
    r2 = barrier(r2 + v4 + v5 + v6);
    v7 = barrier(v7 + r1 + r2 + atomic_val);
    v8 = barrier(v8 + int_from_float + asm_result);
    l1 = barrier(l1 + v7);
    l2 = barrier(l2 + v8);
    l3 = barrier(l3 + sum);
    
    f1 = f1 * 2.0f + (float)l1;
    f2 = f2 * 3.0f + (float)l2;
    d1 = d1 * 1.5 + (double)l3;
    
    /* Convert float back to int through union */
    ns[1].pun.f = f1;
    ns[2].pun.f = f2;
    
    /* Final computation using all variables */
    int result = (r1 ^ r2) + (v1 * v2) - (v3 / (v4|1)) + 
                 (v5 & v6) + (v7 << 2) + (v8 >> 1) +
                 (int)l1 + (int)l2 + (int)(l3 % 1000) +
                 (int)f1 + (int)f2 + (int)d1 +
                 sum + atomic_val + asm_result +
                 ns[0].pun.i + ns[1].pun.i + ns[2].pun.i;
    
    return barrier(result);
}

int main(int argc, char **argv) {
    /* Initialize many variables with non-constant values */
    int vars[30];
    for (int i = 0; i < 30; i++) {
        vars[i] = (i * 37 + argc) & 0xFF;
    }
    
    /* Call test function with many arguments */
    int result = test_reloads(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9]
    );
    
    /* Use remaining variables to prevent dead code elimination */
    for (int i = 10; i < 30; i++) {
        result ^= barrier(vars[i] + i);
    }
    
    /* Print result to ensure observable behavior */
    printf("Result: %d\n", result);
    
    /* Additional complex addressing in main */
    volatile int* array = (volatile int*)malloc(100 * sizeof(int));
    if (array) {
        for (int i = 0; i < 100; i++) {
            /* Complex addressing: array[base + i*stride] */
            int base = global_base % 50;
            int stride = 2;
            array[base + i * stride] = i + result;
        }
        
        /* Force reload with atomic operation */
        __atomic_store_n(&array[50], result, __ATOMIC_RELAXED);
        free((void*)array);
    }
    
    return result & 0xFF;
}
