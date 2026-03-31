/* reload_stress.c - Stress GCC's reload pass to cover reload.cc lines 1381-1399 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    return x ^ 0x55AA55AA;
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
static int multi_array[8][16][4];
static volatile int volatile_index = 3;

/* Inline assembly that clobbers many registers */
#define CLOBBER_MANY_ASM() \
    __asm__ volatile ( \
        "# Clobber many registers\n" \
        "movl $0, %%eax\n" \
        "movl $0, %%ebx\n" \
        "movl $0, %%ecx\n" \
        "movl $0, %%edx\n" \
        "movl $0, %%esi\n" \
        "movl $0, %%edi\n" \
        : \
        : \
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory" \
    )

/* Test function with high register pressure */
__attribute__((noinline, optimize("O1")))
static long test_reloads(int a1, int a2, int a3, int a4, int a5,
                         int a6, int a7, int a8, int a9, int a10) {
    /* Declare many local variables to exhaust registers */
    register int r1 asm ("r12") = a1;
    register int r2 asm ("r13") = a2;
    int v1 = a3, v2 = a4, v3 = a5, v4 = a6, v5 = a7, v6 = a8, v7 = a9, v8 = a10;
    int v9 = barrier(v1), v10 = barrier(v2), v11 = barrier(v3), v12 = barrier(v4);
    int v13 = barrier(v5), v14 = barrier(v6), v15 = barrier(v7), v16 = barrier(v8);
    long l1 = (long)v1 * v2, l2 = (long)v3 * v4, l3 = (long)v5 * v6, l4 = (long)v7 * v8;
    float f1 = (float)v9 / 3.0f, f2 = (float)v10 / 7.0f;
    double d1 = (double)v11 * 1.5, d2 = (double)v12 * 2.5;
    
    /* Force register pressure with arithmetic chain */
    v1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    v2 = v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    v3 = (v1 * v2) / (v1 - v2 + 1);
    v4 = (v3 << 3) | (v2 >> 2);
    v5 = barrier(v3) ^ barrier(v4);
    v6 = (v5 & 0xFF) | ((v5 >> 8) & 0xFF00);
    v7 = v6 * 0x1234567;
    v8 = v7 - v6 + v5 - v4 + v3 - v2 + v1;
    
    /* Complex addressing mode - SIB addressing on x86 */
    int idx1 = volatile_index;
    int idx2 = barrier(idx1) & 7;
    int idx3 = barrier(idx2) & 15;
    
    /* This should require secondary reloads on many architectures */
    int mem_val1 = multi_array[idx1][idx2][idx3];
    int mem_val2 = multi_array[idx2][idx3][idx1];
    int mem_val3 = multi_array[idx3][idx1][idx2];
    
    /* Use inline assembly with memory constraints */
    int asm_out1, asm_out2;
    __asm__ volatile (
        "# Complex addressing with memory constraint\n"
        "movl %[mem1], %%eax\n"
        "addl %[mem2], %%eax\n"
        "movl %%eax, %[out1]\n"
        "movl %[mem3], %%ebx\n"
        "subl %%eax, %%ebx\n"
        "movl %%ebx, %[out2]\n"
        : [out1] "=r" (asm_out1), [out2] "=r" (asm_out2)
        : [mem1] "m" (multi_array[idx1][idx2][idx3]),
          [mem2] "m" (multi_array[idx2][idx3][idx1]),
          [mem3] "m" (multi_array[idx3][idx1][idx2])
        : "eax", "ebx", "memory"
    );
    
    /* Clobber many registers */
    CLOBBER_MANY_ASM();
    
    /* More arithmetic to keep values live */
    l1 = l1 + (long)mem_val1 * asm_out1;
    l2 = l2 + (long)mem_val2 * asm_out2;
    l3 = l3 + (long)mem_val3 * (asm_out1 + asm_out2);
    l4 = l4 + (long)v8 * (mem_val1 + mem_val2 + mem_val3);
    
    /* Mix integer and floating point operations */
    f1 = f1 + (float)asm_out1 / 256.0f;
    f2 = f2 + (float)asm_out2 / 512.0f;
    d1 = d1 + (double)l1 / 1000.0;
    d2 = d2 + (double)l2 / 2000.0;
    
    /* Type punning between int and float */
    union {
        int i;
        float f;
    } pun1, pun2;
    pun1.f = f1;
    pun2.f = f2;
    v9 = pun1.i ^ pun2.i;
    
    /* Atomic operations that may need special handling */
    __atomic_store_n(&multi_array[0][0][0], v9, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&multi_array[1][0][0], __ATOMIC_RELAXED);
    
    /* Use register variables in complex expressions */
    r1 = r1 + v1 + v2 + v3 + v4 + atomic_val;
    r2 = r2 + v5 + v6 + v7 + v8 + v9;
    
    /* Final computation using all variables */
    long result = (long)r1 * r2;
    result += l1 + l2 + l3 + l4;
    result += (long)(f1 * 1000.0f) + (long)(f2 * 1000.0f);
    result += (long)(d1 * 100.0) + (long)(d2 * 100.0);
    result += (long)asm_out1 * asm_out2;
    result += (long)mem_val1 * mem_val2 * mem_val3;
    
    return barrier((int)result) + (result >> 32);
}

/* Another function with different reload patterns */
__attribute__((noinline, optimize("O2")))
static int test_secondary_reloads(struct ComplexData *data, int index) {
    volatile int *volatile_ptr = &data->volatile_member;
    
    /* Force memory-to-register with complex addressing */
    int val1 = data->a[index & 7];
    int val2 = data->a[(index + 1) & 7];
    int val3 = data->a[(index + 2) & 7];
    int val4 = data->a[(index + 3) & 7];
    
    /* Use volatile pointer to prevent optimization */
    int volatile_val = *volatile_ptr;
    
    /* Inline assembly that forces secondary reloads */
    int out1, out2;
    __asm__ volatile (
        "# Force secondary reloads\n"
        "movl %[in1], %%eax\n"
        "leal (%%eax, %%eax, 2), %%ebx\n"  /* ebx = eax * 3 */
        "movl %%ebx, %[out1]\n"
        "movl %[in2], %%ecx\n"
        "imull %%ecx, %%ebx\n"
        "movl %%ebx, %[out2]\n"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in1] "rm" (val1), [in2] "rm" (val2)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* Access with double-register addressing simulation */
    long *long_ptr = &data->b[index & 3];
    long long_val = *long_ptr;
    long_val += (long)val3 * val4;
    long_val += (long)out1 * out2;
    long_val += volatile_val;
    
    /* Mixed-type operations */
    float f = data->c[index & 3];
    f = f * 1.5f + (float)out1 / 128.0f;
    data->c[index & 3] = f;
    
    /* Type punning */
    union {
        float f;
        int i;
    } pun;
    pun.f = f;
    
    return (int)long_val + out1 + out2 + pun.i + volatile_val;
}

int main(int argc, char *argv[]) {
    /* Initialize multi-dimensional array */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 4; k++) {
                multi_array[i][j][k] = i * 1000 + j * 100 + k;
            }
        }
    }
    
    /* Initialize complex structure */
    struct ComplexData data;
    for (int i = 0; i < 8; i++) {
        data.a[i] = i * 10 + argc;
    }
    for (int i = 0; i < 4; i++) {
        data.b[i] = i * 100 + argc;
        data.c[i] = i * 1.5f + argc;
    }
    for (int i = 0; i < 2; i++) {
        data.d[i] = i * 2.5 + argc;
    }
    data.volatile_member = argc * 12345;
    
    /* Create many live variables */
    int x1 = argc * 1, x2 = argc * 2, x3 = argc * 3, x4 = argc * 4, x5 = argc * 5;
    int x6 = argc * 6, x7 = argc * 7, x8 = argc * 8, x9 = argc * 9, x10 = argc * 10;
    int x11 = barrier(x1), x12 = barrier(x2), x13 = barrier(x3), x14 = barrier(x4);
    int x15 = barrier(x5), x16 = barrier(x6), x17 = barrier(x7), x18 = barrier(x8);
    int x19 = barrier(x9), x20 = barrier(x10);
    
    /* Call test function with many arguments */
    long result1 = test_reloads(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10);
    
    /* More operations to keep variables live */
    x1 = x1 + x11 + x12;
    x2 = x2 + x13 + x14;
    x3 = x3 + x15 + x16;
    x4 = x4 + x17 + x18;
    x5 = x5 + x19 + x20;
    
    /* Call second test function */
    int result2 = test_secondary_reloads(&data, argc);
    
    /* Use all variables in final computation */
    long final_result = result1 + result2;
    final_result += x1 + x2 + x3 + x4 + x5;
    final_result += x6 + x7 + x8 + x9 + x10;
    final_result += x11 + x12 + x13 + x14 + x15;
    final_result += x16 + x17 + x18 + x19 + x20;
    
    /* Access volatile memory */
    volatile_index = (argc & 7);
    final_result += multi_array[volatile_index][0][0];
    
    printf("Result: %ld\n", final_result);
    return (final_result > 0) ? 0 : 1;
}
