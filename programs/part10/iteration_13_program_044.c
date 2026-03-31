/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline asm to prevent optimization */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Complex structure with mixed types */
struct nested {
    int a[3];
    long b;
    float c;
    struct nested *next;
};

/* Volatile array with complex addressing */
volatile long volatile_array[256];

/* Global variables to increase register pressure */
int global_counter = 0;

/* Test function with high register pressure */
__attribute__((noinline, optimize("O1")))
long test_reloads(int a1, int a2, int a3, int a4, int a5,
                  int a6, int a7, int a8, int a9, int a10,
                  long l1, long l2, long l3, long l4, long l5,
                  float f1, float f2, double d1, double d2) {
    
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a1;
    register int r1 asm ("r13") = a2;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    long lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8;
    float fv1, fv2, fv3, fv4;
    double dv1, dv2, dv3;
    
    /* Initialize with arithmetic to create dependencies */
    v1 = barrier(a1 + 1);
    v2 = barrier(a2 + v1);
    v3 = barrier(a3 + v2);
    v4 = barrier(a4 + v3);
    v5 = barrier(a5 + v4);
    v6 = barrier(a6 + v5);
    v7 = barrier(a7 + v6);
    v8 = barrier(a8 + v7);
    v9 = barrier(a9 + v8);
    v10 = barrier(a10 + v9);
    
    v11 = barrier(v1 * 2);
    v12 = barrier(v2 * 3);
    v13 = barrier(v3 * 5);
    v14 = barrier(v4 * 7);
    v15 = barrier(v5 * 11);
    v16 = barrier(v6 * 13);
    v17 = barrier(v7 * 17);
    v18 = barrier(v8 * 19);
    v19 = barrier(v9 * 23);
    v20 = barrier(v10 * 29);
    
    /* Long variables with mixed operations */
    lv1 = l1 + v1;
    lv2 = l2 + v2 + lv1;
    lv3 = l3 + v3 + lv2;
    lv4 = l4 + v4 + lv3;
    lv5 = l5 + v5 + lv4;
    lv6 = lv1 * lv2 - lv3;
    lv7 = lv4 * lv5 - lv6;
    lv8 = lv7 + (lv6 >> 3);
    
    /* Floating point operations to use FP registers */
    fv1 = f1 + 1.0f;
    fv2 = f2 + fv1;
    fv3 = fv1 * fv2 - 3.14f;
    fv4 = fv3 / fv2 + f1;
    
    dv1 = d1 + 2.0;
    dv2 = d2 + dv1;
    dv3 = dv1 * dv2 - 3.14159;
    
    /* Complex array access with SIB addressing (x86) */
    volatile int idx1 = barrier(v1) & 0xFF;
    volatile int idx2 = barrier(v2) & 0xFF;
    volatile int idx3 = barrier(v3) & 0xFF;
    
    /* Multi-dimensional array access with complex addressing */
    int md_array[32][32];
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            md_array[i][j] = i * 32 + j;
        }
    }
    
    /* Complex addressing: array[base + index*scale] */
    int scale = 4;
    int base = idx1;
    int index = idx2;
    
    /* This should require secondary reload on some arches */
    int complex_addr_value = md_array[base + index * scale][idx3];
    
    /* Inline asm that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %[tmp1]\n\t"
        "add %[val2], %[tmp1]\n\t"
        "mov %[tmp1], %[out1]\n\t"
        : [out1] "=r" (v1)
        : [val1] "m" (md_array[base][index]), 
          [val2] "r" (v2),
          [tmp1] "r" (r0)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* More complex inline asm with memory constraints */
    long temp_result;
    __asm__ volatile (
        "# Memory operand with complex address\n"
        "ldr %[res], [%[addr], %[idx], lsl #2]\n\t"
        : [res] "=r" (temp_result)
        : [addr] "r" (md_array),
          [idx] "r" (idx1 * 32 + idx2)
        : "memory"
    );
    
    /* Atomic operations that may need special handling */
    _Atomic int atomic_var = 42;
    int old_val = __atomic_exchange_n(&atomic_var, v1 + v2, __ATOMIC_RELAXED);
    
    /* Type punning between int and float */
    union {
        int i;
        float f;
    } punner;
    
    punner.f = fv1;
    int int_from_float = punner.i + v3;
    punner.i = v4;
    float float_from_int = punner.f + fv2;
    
    /* Access volatile array with complex index */
    volatile long vol_val = volatile_array[idx1 * 8 + idx2 * 2 + idx3];
    
    /* Use register variables in complex expressions */
    r0 = r0 + r1 + v1 + v2 + v3;
    r1 = r1 * 2 - r0;
    
    /* Function call to force spills */
    int call_result = barrier(r0 + r1);
    
    /* Complex structure access */
    struct nested nested_array[16];
    for (int i = 0; i < 16; i++) {
        nested_array[i].a[0] = i;
        nested_array[i].a[1] = i * 2;
        nested_array[i].a[2] = i * 3;
        nested_array[i].b = i * 100L;
        nested_array[i].c = i * 1.5f;
        nested_array[i].next = &nested_array[(i + 1) % 16];
    }
    
    /* Complex pointer chain access */
    int struct_sum = 0;
    struct nested *ptr = &nested_array[idx1 % 16];
    for (int i = 0; i < 8; i++) {
        struct_sum += ptr->a[i % 3];
        struct_sum += ptr->b;
        ptr = ptr->next;
    }
    
    /* Final computation using all variables */
    long final_result = 
        (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        lv1 + lv2 + lv3 + lv4 + lv5 + lv6 + lv7 + lv8 +
        (long)fv1 + (long)fv2 + (long)fv3 + (long)fv4 +
        (long)dv1 + (long)dv2 + (long)dv3 +
        complex_addr_value + temp_result + old_val +
        int_from_float + (long)float_from_int + vol_val +
        call_result + struct_sum + r0 + r1;
    
    return final_result;
}

int main(int argc, char *argv[]) {
    /* Initialize volatile array */
    for (int i = 0; i < 256; i++) {
        volatile_array[i] = i * 3L;
    }
    
    /* Create many live variables */
    int a1 = barrier(argc + 1);
    int a2 = barrier(argc + 2);
    int a3 = barrier(argc + 3);
    int a4 = barrier(argc + 4);
    int a5 = barrier(argc + 5);
    int a6 = barrier(argc + 6);
    int a7 = barrier(argc + 7);
    int a8 = barrier(argc + 8);
    int a9 = barrier(argc + 9);
    int a10 = barrier(argc + 10);
    
    long l1 = barrier(argc + 100);
    long l2 = barrier(argc + 200);
    long l3 = barrier(argc + 300);
    long l4 = barrier(argc + 400);
    long l5 = barrier(argc + 500);
    
    float f1 = barrier(argc) * 1.1f;
    float f2 = barrier(argc) * 2.2f;
    
    double d1 = barrier(argc) * 3.14159;
    double d2 = barrier(argc) * 2.71828;
    
    /* Call test function multiple times with different args */
    long total = 0;
    for (int i = 0; i < 10; i++) {
        long result = test_reloads(
            a1 + i, a2 + i, a3 + i, a4 + i, a5 + i,
            a6 + i, a7 + i, a8 + i, a9 + i, a10 + i,
            l1 + i, l2 + i, l3 + i, l4 + i, l5 + i,
            f1 + i, f2 + i, d1 + i, d2 + i
        );
        total += result;
        
        /* Modify some variables to prevent optimization */
        a1 = barrier(a1 + result);
        a2 = barrier(a2 + result);
    }
    
    printf("Result: %ld\n", total);
    return (int)(total % 1000);
}
