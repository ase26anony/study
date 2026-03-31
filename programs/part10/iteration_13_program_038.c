/* reload_stress.c - Stress GCC's reload pass to cover reload.cc lines 1381-1399 */

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent register reuse */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to prevent optimization */
    __asm__ volatile ("" : "+r" (x) : : "memory");
    return x;
}

/* Complex structure with mixed types */
struct ComplexData {
    int a[8];
    long b[4];
    float c[4];
    double d[2];
    volatile int volatile_member;
};

/* Multi-dimensional array */
static int multi_array[8][8][8];

/* Global volatile to force memory operations */
volatile int global_volatile = 42;

/* Test function with many parameters to force register pressure */
__attribute__((noinline, optimize("O1")))
long test_function(
    int p1, int p2, int p3, int p4, int p5,
    int p6, int p7, int p8, int p9, int p10,
    long p11, long p12, long p13, long p14,
    float p15, float p16, double p17, double p18,
    struct ComplexData* data, volatile int* volatile_ptr
) {
    /* Declare many local variables to exhaust registers */
    register int r1 asm ("r12") = p1;
    register int r2 asm ("r13") = p2;
    int v1 = p3, v2 = p4, v3 = p5, v4 = p6, v5 = p7;
    int v6 = p8, v7 = p9, v8 = p10;
    long l1 = p11, l2 = p12, l3 = p13, l4 = p14;
    float f1 = p15, f2 = p16;
    double d1 = p17, d2 = p18;
    
    /* Complex arithmetic with many dependencies */
    r1 = barrier(r1 + v1 * 3 - v2 / 2);
    r2 = barrier(r2 ^ v3 | v4 & 0xFF);
    v1 = v1 + v5 - v6 * r1;
    v2 = v2 + v7 / (r2 ? r2 : 1);
    v3 = v3 ^ v8 ^ r1;
    v4 = v4 | (v1 << 2);
    v5 = v5 & (v2 >> 1);
    v6 = barrier(v6 + v3 * 2);
    v7 = barrier(v7 - v4 / 3);
    v8 = barrier(v8 ^ v5 | 0x55);
    
    /* Force floating-point operations to use FP registers */
    f1 = f1 * 1.5f + f2;
    f2 = barrier((int)f2) * 0.5f;  /* Type punning */
    d1 = d1 / 2.0 + d2;
    d2 = barrier((long)d2) * 1.25; /* Type punning */
    
    /* Long integer operations */
    l1 = l1 + l2 * l3 - l4;
    l2 = l2 ^ l1 | l3 & 0xFFFF;
    l3 = barrier(l3 + l4 / (l1 ? l1 : 1));
    l4 = l4 - l2 * 3 + l1;
    
    /* Inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "movl %0, %%eax\n"
        "movl %1, %%ebx\n"
        "addl %%ebx, %%eax\n"
        "movl %%eax, %0\n"
        : "+r" (v1), "+r" (v2)
        : 
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Complex memory addressing with SIB (Scale-Index-Base) */
    volatile int idx1 = barrier(v3) & 7;
    volatile int idx2 = barrier(v4) & 7;
    volatile int idx3 = barrier(v5) & 7;
    
    /* This should require complex addressing mode */
    int array_val = multi_array[idx1][idx2][idx3];
    array_val += barrier(array_val);
    
    /* More complex addressing with structure */
    int struct_idx = barrier(v6) & 7;
    data->a[struct_idx] = barrier(data->a[struct_idx] + v7);
    
    /* Volatile memory operations */
    *volatile_ptr = barrier(*volatile_ptr + v8);
    data->volatile_member = barrier(data->volatile_member + r1);
    
    /* Atomic operations with memory ordering */
    int atomic_temp = __atomic_load_n(volatile_ptr, __ATOMIC_RELAXED);
    __atomic_store_n(&data->a[0], atomic_temp + r2, __ATOMIC_RELAXED);
    
    /* Mixed register class operations */
    union {
        float f;
        int i;
    } punner;
    punner.f = f1;
    v1 = barrier(v1 + punner.i);  /* Force move between FP and GP registers */
    
    /* Another inline assembly with memory constraint */
    int mem_temp;
    __asm__ volatile (
        "# Memory constraint with complex address\n"
        "movl (%1, %2, 4), %0\n"  /* SIB addressing: data->a[index*4] */
        : "=r" (mem_temp)
        : "r" (data->a), "r" (struct_idx)
        : "memory"
    );
    v2 = barrier(v2 + mem_temp);
    
    /* Create secondary reload scenario */
    register long complex_addr asm ("r14") = (long)&data->b[0] + l1 * 8;
    long loaded_val;
    __asm__ volatile (
        "# Force secondary reload\n"
        "movq (%1), %0\n"
        : "=r" (loaded_val)
        : "r" (complex_addr)
        : "memory"
    );
    l1 = barrier(l1 + loaded_val);
    
    /* Compute checksum from all modified values */
    long checksum = r1 + r2 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                   l1 + l2 + l3 + l4 + (int)f1 + (int)f2 + (long)d1 + (long)d2 +
                   array_val + data->a[0] + *volatile_ptr + data->volatile_member +
                   mem_temp + loaded_val;
    
    return barrier(checksum);
}

int main(int argc, char *argv[]) {
    /* Initialize multi-dimensional array */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                multi_array[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    /* Initialize complex structure */
    struct ComplexData data;
    for (int i = 0; i < 8; i++) data.a[i] = i * 10;
    for (int i = 0; i < 4; i++) {
        data.b[i] = i * 100L;
        data.c[i] = i * 1.5f;
    }
    for (int i = 0; i < 2; i++) data.d[i] = i * 3.14;
    data.volatile_member = 999;
    
    /* Create many scalar variables to increase register pressure */
    int var1 = barrier(argc + 1);
    int var2 = barrier(argc + 2);
    int var3 = barrier(argc + 3);
    int var4 = barrier(argc + 4);
    int var5 = barrier(argc + 5);
    int var6 = barrier(argc + 6);
    int var7 = barrier(argc + 7);
    int var8 = barrier(argc + 8);
    int var9 = barrier(argc + 9);
    int var10 = barrier(argc + 10);
    int var11 = barrier(argc + 11);
    int var12 = barrier(argc + 12);
    int var13 = barrier(argc + 13);
    int var14 = barrier(argc + 14);
    int var15 = barrier(argc + 15);
    int var16 = barrier(argc + 16);
    int var17 = barrier(argc + 17);
    int var18 = barrier(argc + 18);
    int var19 = barrier(argc + 19);
    int var20 = barrier(argc + 20);
    
    long lvar1 = barrier(argc + 100);
    long lvar2 = barrier(argc + 200);
    long lvar3 = barrier(argc + 300);
    long lvar4 = barrier(argc + 400);
    
    float fvar1 = barrier(argc) * 1.1f;
    float fvar2 = barrier(argc) * 2.2f;
    double dvar1 = barrier(argc) * 3.3;
    double dvar2 = barrier(argc) * 4.4;
    
    volatile int volatile_var = global_volatile;
    
    /* Call test function multiple times with different arguments */
    long result1 = test_function(
        var1, var2, var3, var4, var5,
        var6, var7, var8, var9, var10,
        lvar1, lvar2, lvar3, lvar4,
        fvar1, fvar2, dvar1, dvar2,
        &data, &volatile_var
    );
    
    /* Modify variables and call again */
    var1 = barrier(var1 + 1);
    var2 = barrier(var2 + 2);
    lvar1 = barrier(lvar1 + 100);
    fvar1 = barrier((int)fvar1) * 1.5f;
    
    long result2 = test_function(
        var11, var12, var13, var14, var15,
        var16, var17, var18, var19, var20,
        lvar1, lvar2, lvar3, lvar4,
        fvar1, fvar2, dvar1, dvar2,
        &data, &volatile_var
    );
    
    /* Final computation using all results */
    long final_result = barrier(result1 + result2 + volatile_var + data.volatile_member);
    
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
