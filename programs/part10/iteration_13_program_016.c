/* reload_test.c - Test program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    volatile int y = x;
    return y ^ 0x55;
}

/* Complex structure to force complex addressing */
struct nested {
    int a[4];
    struct {
        long b[3];
        float c[2];
    } inner;
    volatile int d;
};

/* Multi-dimensional array with volatile indices */
volatile int idx1 = 1, idx2 = 2, idx3 = 3;

/* Inline assembly that clobbers many registers */
#define CLOBBER_ASM() __asm__ volatile ( \
    "# Clobber many registers\n" \
    "movl $0, %%eax\n" \
    "movl $0, %%ebx\n" \
    "movl $0, %%ecx\n" \
    "movl $0, %%edx\n" \
    "movl $0, %%esi\n" \
    "movl $0, %%edi\n" \
    : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory")

/* Test function with many register pressures */
__attribute__((noinline, optimize("O1")))
static long test_reloads(int a1, int a2, int a3, int a4, int a5,
                         int a6, int a7, int a8, int a9, int a10,
                         long b1, long b2, long b3, long b4, long b5,
                         float f1, float f2, float f3) {
    /* Declare many local variables to exhaust registers */
    register int r1 asm ("r12") = a1;
    register int r2 asm ("r13") = a2;
    register int r3 asm ("r14") = a3;
    int l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    int m1, m2, m3, m4, m5, m6, m7, m8, m9, m10;
    long n1, n2, n3, n4, n5, n6, n7, n8;
    float g1, g2, g3, g4, g5;
    volatile int v1, v2, v3;
    
    /* Complex array with SIB addressing */
    int array[256];
    struct nested nested_array[8];
    volatile int *volatile ptr = &v1;
    
    /* Initialize with arithmetic to create dependencies */
    l1 = a1 + a2; l2 = a3 * a4; l3 = a5 ^ a6; l4 = a7 | a8;
    l5 = a9 & a10; l6 = l1 + l2; l7 = l3 - l4; l8 = l5 * l6;
    l9 = l7 / (l1 ? l1 : 1); l10 = l8 % (l2 ? l2 : 1);
    
    m1 = b1 + b2; m2 = b3 * b4; m3 = b5 + m1; m4 = m2 - m3;
    m5 = m4 ^ m1; m6 = m5 | m2; m7 = m6 & m3; m8 = m7 + m4;
    m9 = m8 * m5; m10 = m9 / (m6 ? m6 : 1);
    
    /* Force spills with many live variables */
    n1 = (long)l1 * (long)l2;
    n2 = (long)l3 + (long)l4;
    n3 = (long)l5 ^ (long)l6;
    n4 = (long)l7 | (long)l8;
    n5 = (long)l9 & (long)l10;
    n6 = n1 + n2;
    n7 = n3 * n4;
    n8 = n5 - n6;
    
    /* Mixed float/int operations */
    g1 = f1 + f2;
    g2 = f3 * g1;
    g3 = (float)l1 + f1;
    g4 = (float)l2 * f2;
    g5 = g3 - g4;
    
    /* Complex addressing with SIB (Scale-Index-Base) */
    /* This should force secondary reloads on x86 */
    int base = idx1;
    int index = idx2;
    int scale = idx3;
    
    /* Force complex memory addressing */
    for (int i = 0; i < 8; i++) {
        /* SIB addressing: array[base + index * scale] */
        array[base + index * scale] = l1 + i;
        base = barrier(base);
        index = barrier(index);
        
        /* Nested structure access with complex addressing */
        nested_array[i].a[(i + idx1) % 4] = l2 + i;
        nested_array[i].inner.b[(i + idx2) % 3] = n1 + i;
        nested_array[i].inner.c[(i + idx3) % 2] = g1 + i;
        nested_array[i].d = barrier(i);
    }
    
    /* Inline assembly with complex constraints */
    /* This should create multiple reload records */
    int temp1, temp2, temp3;
    volatile int mem_var = 0x12345678;
    
    __asm__ volatile (
        "# Complex inline assembly with memory constraints\n"
        "movl %[mem], %%eax\n"
        "addl %%eax, %[out1]\n"
        "movl %[in1], %%ebx\n"
        "imull %%ebx, %[out2]\n"
        : [out1] "=r" (temp1), [out2] "=r" (temp2)
        : [mem] "m" (mem_var), [in1] "r" (l1),
          "[out1]" (temp1), "[out2]" (temp2)
        : "eax", "ebx", "memory"
    );
    
    /* More complex asm with multiple outputs */
    long long_temp;
    __asm__ volatile (
        "# Another asm with register constraints\n"
        "mov %[in2], %[out3]\n"
        "add $0x1000, %[out3]\n"
        : [out3] "=r" (long_temp)
        : [in2] "r" (n8)
        : "cc"
    );
    
    /* Clobber many registers between computations */
    CLOBBER_ASM();
    
    /* Continue computation with clobbered registers */
    temp3 = barrier(temp1) + barrier(temp2);
    
    /* Atomic operations that may need special handling */
    __atomic_store_n(&v1, temp3, __ATOMIC_RELAXED);
    v2 = __atomic_load_n(&v1, __ATOMIC_RELAXED);
    
    /* Type punning between int and float */
    union {
        int i;
        float f;
    } pun;
    pun.i = v2;
    g1 = pun.f * 2.0f;
    pun.f = g1;
    v3 = pun.i;
    
    /* Final computation using all variables */
    long result = (long)l1 + (long)l2 + (long)l3 + (long)l4 + (long)l5 +
                  (long)l6 + (long)l7 + (long)l8 + (long)l9 + (long)l10 +
                  m1 + m2 + m3 + m4 + m5 + m6 + m7 + m8 + m9 + m10 +
                  n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 +
                  (long)temp1 + (long)temp2 + (long)temp3 +
                  (long)v1 + (long)v2 + (long)v3 +
                  (long)(g1 * 100.0f) + (long)(g2 * 100.0f) +
                  (long)(g3 * 100.0f) + (long)(g4 * 100.0f) +
                  (long)(g5 * 100.0f) +
                  (long)r1 + (long)r2 + (long)r3 +
                  (long)array[idx1 + idx2 * idx3] +
                  (long)nested_array[idx1].inner.b[idx2];
    
    return barrier(result);
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
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
    
    long b1 = (long)barrier(argc + 11) * 1000L;
    long b2 = (long)barrier(argc + 12) * 2000L;
    long b3 = (long)barrier(argc + 13) * 3000L;
    long b4 = (long)barrier(argc + 14) * 4000L;
    long b5 = (long)barrier(argc + 15) * 5000L;
    
    float f1 = (float)barrier(argc + 16) / 100.0f;
    float f2 = (float)barrier(argc + 17) / 200.0f;
    float f3 = (float)barrier(argc + 18) / 300.0f;
    
    /* Call test function multiple times with different args */
    long result1 = test_reloads(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                               b1, b2, b3, b4, b5, f1, f2, f3);
    
    /* Modify some values and call again */
    a1 = barrier(a1 + 1);
    a2 = barrier(a2 + 2);
    b1 = b1 + 10000L;
    f1 = f1 * 2.0f;
    
    long result2 = test_reloads(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                               b1, b2, b3, b4, b5, f1, f2, f3);
    
    /* Final checksum */
    long final_result = barrier(result1) + barrier(result2);
    
    printf("Result: %ld\n", final_result);
    
    /* Use result to prevent dead code elimination */
    if (final_result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return (final_result != 0) ? 0 : 1;
}

/* Implementation of barrier function */
int barrier(int x) {
    static volatile int counter = 0;
    counter++;
    return x ^ (counter * 0x5A5A5A5A);
}
