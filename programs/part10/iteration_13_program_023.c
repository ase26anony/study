/* reload_stress.c - Stress GCC's reload pass to cover reload.cc lines 1381-1399 */

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Implementation for linking */
    return x ^ 0x55AA55AA;
}

/* Complex structure with mixed types */
struct ComplexData {
    int a;
    long b;
    float c;
    double d;
    int* e;
    volatile int f;
};

/* Multi-dimensional array */
static int multi_array[8][16][32];

/* Global volatile to force memory operations */
volatile int g_volatile = 12345;

/* Function with many register pressures */
__attribute__((noinline, optimize("O0")))
long test_reload_stress(int p1, int p2, int p3, int p4, int p5,
                       int p6, int p7, int p8, int p9, int p10,
                       long p11, long p12, long p13, long p14, long p15) {
    
    /* Declare many local variables to exhaust registers */
    register int r1 asm ("r12") = p1 + 1;
    register int r2 asm ("r13") = p2 + 2;
    int v1 = p3 * 3;
    int v2 = p4 / 4;
    int v3 = p5 ^ 0xFF;
    int v4 = p6 | 0xAA;
    int v5 = p7 & 0x55;
    int v6 = p8 << 2;
    int v7 = p9 >> 1;
    int v8 = p10 - 100;
    long l1 = p11 * 11;
    long l2 = p12 + 22;
    long l3 = p13 - 33;
    long l4 = p14 | 0xCC;
    long l5 = p15 & 0x33;
    
    /* Force spills with arithmetic chain */
    v1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    v2 = v2 * v1 - v3;
    v3 = v3 ^ v2 | v4;
    v4 = v4 + v3 * v5;
    v5 = v5 - v4 / (v6 ? v6 : 1);
    v6 = v6 << (v7 & 3);
    v7 = v7 >> (v8 & 3);
    v8 = v8 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7;
    
    l1 = l1 + l2 + l3 + l4 + l5;
    l2 = l2 * l1 - l3;
    l3 = l3 ^ l2 | l4;
    l4 = l4 + l3 * l5;
    l5 = l5 - l4 / (l1 ? l1 : 1);
    
    /* Complex addressing with SIB-like calculation (for x86) */
    volatile int idx1 = (v1 & 7);
    volatile int idx2 = (v2 & 15);
    volatile int idx3 = (v3 & 31);
    
    /* Force secondary reloads with complex memory addressing */
    int array_val = multi_array[idx1][idx2][idx3];
    array_val += multi_array[idx2][idx3][idx1];
    array_val += multi_array[idx3][idx1][idx2];
    
    /* Use the value in further computation */
    v1 = v1 + array_val;
    
    /* Inline assembly that clobbers many registers */
    asm volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %%eax\n"
        "mov %[val2], %%ebx\n"
        "add %%ebx, %%eax\n"
        "mov %%eax, %[result]\n"
        : [result] "=m" (v2)  /* Memory output constraint */
        : [val1] "rm" (v1),   /* Register or memory input */
          [val2] "rm" (array_val)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* Mixed register classes - integer to float */
    float f1 = (float)v3;
    double d1 = (double)v4;
    int int_from_float = *(int*)&f1;  /* Type punning */
    
    /* Atomic operations with memory constraints */
    _Atomic int atomic_var = 0;
    __atomic_store_n(&atomic_var, v5, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* More complex addressing with structure */
    struct ComplexData data;
    data.a = v6;
    data.b = l1;
    data.c = f1;
    data.d = d1;
    data.e = &v7;
    data.f = g_volatile;
    
    /* Access structure with volatile member */
    int struct_val = data.f + data.a;
    
    /* Another inline asm with explicit register variable */
    int final_result;
    asm volatile (
        "# Use register variable\n"
        "addl %[reg1], %[reg2]\n"
        "movl %[reg2], %[out]\n"
        : [out] "=r" (final_result)
        : [reg1] "r" (r1), [reg2] "r" (r2)
        : "cc"
    );
    
    /* Long dependency chain continues */
    final_result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    final_result += l1 + l2 + l3 + l4 + l5;
    final_result += atomic_val;
    final_result += struct_val;
    final_result += int_from_float;
    final_result += barrier(final_result);
    
    return final_result;
}

int main(int argc, char *argv[]) {
    /* Initialize multi-dimensional array */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 32; k++) {
                multi_array[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
    
    /* Create many live variables */
    int var1 = argc > 1 ? atoi(argv[1]) : 1;
    int var2 = var1 * 2;
    int var3 = var2 + 3;
    int var4 = var3 - 4;
    int var5 = var4 ^ 0x55;
    int var6 = var5 | 0xAA;
    int var7 = var6 & 0xFF;
    int var8 = var7 << 1;
    int var9 = var8 >> 2;
    int var10 = var9 * 3;
    
    long var11 = (long)var1 * 100;
    long var12 = var11 + 200;
    long var13 = var12 - 300;
    long var14 = var13 | 0x1234;
    long var15 = var14 & 0xABCD;
    
    /* Call test function multiple times with different args */
    long result1 = test_reload_stress(var1, var2, var3, var4, var5,
                                     var6, var7, var8, var9, var10,
                                     var11, var12, var13, var14, var15);
    
    /* Modify variables and call again */
    var1 = barrier(var1);
    var2 = barrier(var2);
    
    long result2 = test_reload_stress(var2, var1, var3, var4, var5,
                                     var6, var7, var8, var9, var10,
                                     var15, var14, var13, var12, var11);
    
    long final_result = result1 + result2;
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
