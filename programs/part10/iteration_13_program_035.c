/* reload_stress.c - Stress GCC's reload pass to cover rld initialization block */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    volatile int v = x;
    return v + 1;
}

/* Complex structure with mixed types */
struct ComplexData {
    int a;
    long b;
    float c;
    double d;
    int arr[8];
    volatile int volatile_member;
};

/* Global arrays to force complex addressing */
volatile long global_array[256];
struct ComplexData data_array[16];
int multi_dim[8][8][8];

/* Force register pressure with many live variables */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
int test_reload_stress(int seed) {
    /* Declare many scalar variables to exhaust registers */
    register int r0 asm ("r12") = seed + 1;
    register int r1 asm ("r13") = seed + 2;
    int v1 = barrier(seed);
    int v2 = barrier(v1);
    int v3 = barrier(v2);
    int v4 = barrier(v3);
    int v5 = barrier(v4);
    int v6 = barrier(v5);
    int v7 = barrier(v6);
    int v8 = barrier(v7);
    int v9 = barrier(v8);
    int v10 = barrier(v9);
    long l1 = v1 * 2L;
    long l2 = v2 * 3L;
    long l3 = v3 * 4L;
    long l4 = v4 * 5L;
    long l5 = v5 * 6L;
    
    /* Mixed integer/float operations to engage different register classes */
    float f1 = (float)v1 / 2.0f;
    float f2 = (float)v2 / 3.0f;
    double d1 = (double)l1 / 2.0;
    double d2 = (double)l2 / 3.0;
    
    /* Complex addressing with SIB-like computation (for x86) */
    volatile int idx1 = v1 & 7;
    volatile int idx2 = v2 & 7;
    volatile int idx3 = v3 & 7;
    
    /* Force multiple memory accesses with complex addressing */
    int sum = 0;
    
    /* Access multi-dimensional array with volatile indices */
    sum += multi_dim[idx1][idx2][idx3];
    sum += multi_dim[idx2][idx3][idx1];
    sum += multi_dim[idx3][idx1][idx2];
    
    /* Access structure with mixed types */
    sum += data_array[idx1].a;
    sum += data_array[idx2].arr[idx3];
    data_array[idx1].volatile_member = v4;
    
    /* Inline assembly that clobbers many registers */
    asm volatile (
        "# Complex inline assembly to force reloads\n"
        "mov %[val1], %[tmp1]\n\t"
        "mov %[val2], %[tmp2]\n\t"
        "add %[tmp1], %[tmp2]\n\t"
        "mov %[tmp2], %[out1]\n\t"
        : [out1] "=r" (v1), [tmp2] "=&r" (v2)
        : [val1] "mr" (global_array[idx1 * 4 + idx2]), 
          [val2] "mr" (global_array[idx2 * 4 + idx3]),
          [tmp1] "r" (r0)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* More arithmetic to create dependency chain */
    v1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    l1 = l1 + l2 + l3 + l4 + l5 + v1;
    
    /* Type punning between int and float */
    union {
        int i;
        float f;
    } punner;
    punner.i = v1;
    f1 = punner.f + f1 + f2;
    punner.f = f1;
    v2 = punner.i;
    
    /* Atomic operations with memory ordering */
    int atomic_val = 0;
    __atomic_store_n(&data_array[idx1].arr[idx2], v3, __ATOMIC_RELAXED);
    atomic_val = __atomic_load_n(&data_array[idx2].arr[idx3], __ATOMIC_ACQUIRE);
    
    /* Complex addressing with scale and index */
    for (int i = 0; i < 8; i++) {
        /* array[base + index*scale] addressing */
        int* addr = &data_array[i].arr[idx1 * 2 + idx2];
        sum += *addr + atomic_val;
        
        /* Force spill by using all variables */
        v1 = v1 + v2 + barrier(v3);
        v2 = v2 + v3 + barrier(v4);
        v3 = v3 + v4 + barrier(v5);
        v4 = v4 + v5 + barrier(v6);
        v5 = v5 + v6 + barrier(v7);
        v6 = v6 + v7 + barrier(v8);
        v7 = v7 + v8 + barrier(v9);
        v8 = v8 + v9 + barrier(v10);
        v9 = v9 + v10 + barrier(v1);
        v10 = v10 + v1 + barrier(v2);
    }
    
    /* Final computation using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += l1 + l2 + l3 + l4 + l5;
    result += (int)f1 + (int)f2 + (int)d1 + (int)d2;
    result += sum + atomic_val + r0 + r1;
    
    return result;
}

int main(int argc, char* argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        data_array[i].a = i * 2;
        data_array[i].b = i * 5L;
        data_array[i].c = i * 1.5f;
        data_array[i].d = i * 2.5;
        for (int j = 0; j < 8; j++) {
            data_array[i].arr[j] = i + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                multi_dim[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    /* Call test function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        int seed = (argc > 1) ? atoi(argv[1]) + i : i * 12345;
        total += test_reload_stress(seed);
    }
    
    printf("Result: %d\n", total);
    return total & 255;
}
