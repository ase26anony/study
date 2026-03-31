/* reload_stress_test.c
 * Stress test for GCC's reload pass initialization block
 * Compile with: gcc -O1 -fschedule-insns -fno-omit-frame-pointer -m32 -march=i686 -fno-pic -fno-optimize-sibling-calls -fno-crossjumping reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __attribute__((noinline));
int barrier(int x) {
    volatile int sink = x;
    return sink + 1;
}

/* Complex structure to force complex addressing */
struct nested {
    int a[3];
    long b[2];
    struct {
        short c;
        int d;
    } inner;
};

/* Multi-dimensional array with volatile indices */
volatile int idx1 = 1, idx2 = 2, idx3 = 3;

/* Inline assembly that clobbers many registers */
#define CLOBBER_ASM() __asm__ volatile ( \
    "# Clobber many registers\n\t" \
    "movl $0, %%eax\n\t" \
    "movl $0, %%ebx\n\t" \
    "movl $0, %%ecx\n\t" \
    "movl $0, %%edx\n\t" \
    "movl $0, %%esi\n\t" \
    "movl $0, %%edi\n\t" \
    : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory")

/* Test function with high register pressure */
__attribute__((noinline, optimize("O0")))
int test_reloads(int start) {
    /* Declare many scalar variables to exhaust registers */
    register int r0 asm ("ebx") = start + 1;
    register int r1 asm ("esi") = start + 2;
    register int r2 asm ("edi") = start + 3;
    
    int v1 = barrier(start);
    int v2 = barrier(v1);
    int v3 = barrier(v2);
    int v4 = barrier(v3);
    int v5 = barrier(v4);
    int v6 = barrier(v5);
    int v7 = barrier(v6);
    int v8 = barrier(v7);
    int v9 = barrier(v8);
    int v10 = barrier(v9);
    int v11 = barrier(v10);
    int v12 = barrier(v11);
    int v13 = barrier(v12);
    int v14 = barrier(v13);
    int v15 = barrier(v14);
    int v16 = barrier(v15);
    int v17 = barrier(v16);
    int v18 = barrier(v17);
    int v19 = barrier(v18);
    int v20 = barrier(v19);
    
    /* Complex array access with SIB addressing */
    volatile int array[256][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Force SIB addressing: array[base + index*scale] */
            array[i * 8 + j][i] = v1 + i * j;
        }
    }
    
    /* Mixed integer operations creating dependency chain */
    v1 = v1 + v2 * v3 - v4 / (v5 | 1);
    v2 = v2 ^ v6 & v7 | v8;
    v3 = v3 * v9 - v10 + v11;
    v4 = (v4 << 2) | (v12 >> 3);
    v5 = v5 + v13 - v14 * v15;
    v6 = v6 & v16 | v17 ^ v18;
    v7 = v7 * v19 / (v20 | 1);
    
    /* Use register variables in complex expressions */
    r0 = r0 + v1 * r1 - v2 / (r2 | 1);
    r1 = r1 ^ v3 & r0 | v4;
    r2 = r2 * v5 - v6 + r1;
    
    /* Inline assembly with memory constraints forcing reloads */
    int temp;
    __asm__ volatile (
        "# Complex addressing with memory constraint\n\t"
        "movl %[addr], %[temp]\n\t"
        "addl $1, %[temp]\n\t"
        "movl %[temp], %[addr]\n\t"
        : [temp] "=&r" (temp), [addr] "=m" (array[idx1 * 8 + idx2][idx3])
        : 
        : "memory"
    );
    
    /* Force clobbering of many registers */
    CLOBBER_ASM();
    
    /* More arithmetic to use all variables */
    v8 = v8 + r0 - r1 * r2;
    v9 = v9 ^ v10 & v11 | v12;
    v10 = v10 * v13 - v14 + v15;
    v11 = (v11 << 3) | (v16 >> 2);
    v12 = v12 + v17 - v18 * v19;
    v13 = v13 & v20 | v1 ^ v2;
    v14 = v14 * v3 / (v4 | 1);
    
    /* Access complex structure with volatile pointer */
    volatile struct nested nested_array[4];
    volatile int *volatile ptr = &nested_array[idx1].inner.d;
    
    /* Force reload through atomic operation */
    __atomic_store_n(ptr, v5 + v6 + v7, __ATOMIC_RELAXED);
    
    /* Mixed floating point operations to engage different register classes */
    float f1 = (float)v8;
    float f2 = (float)v9;
    float f3 = f1 * f2 - (float)v10;
    
    /* Type punning through union */
    union {
        float f;
        int i;
    } pun;
    pun.f = f3;
    v15 = pun.i + v11;
    
    /* More clobbering */
    CLOBBER_ASM();
    
    /* Final computation using all variables */
    int result = r0 + r1 + r2 +
                 v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 +
                 array[0][0] + *ptr;
    
    return result & 0x7FFFFFFF; /* Keep positive */
}

/* Secondary test for specific secondary reload patterns */
__attribute__((noinline))
int test_secondary_reloads(int base) {
    volatile long data[100];
    
    /* Initialize with pattern */
    for (int i = 0; i < 100; i++) {
        data[i] = base + i * 3;
    }
    
    /* Complex addressing that may need secondary reload */
    register long sum asm ("ebx") = 0;
    
    for (int i = 1; i < 99; i++) {
        /* Access with complex index: data[base + i*scale + offset] */
        /* This may require SIB addressing with reloads on x86 */
        long val1 = data[i * 2 + 1];
        long val2 = data[i * 3 - 2];
        
        /* Inline asm with explicit register constraints */
        __asm__ volatile (
            "# Force register/memory moves\n\t"
            "addl %[v1], %[sum]\n\t"
            "addl %[v2], %[sum]\n\t"
            : [sum] "+r" (sum)
            : [v1] "rm" (val1), [v2] "rm" (val2)
            : "cc"
        );
    }
    
    /* Use the register variable in memory access */
    volatile long *volatile ptr = &data[0];
    for (int i = 0; i < 10; i++) {
        __atomic_fetch_add(ptr + i, sum, __ATOMIC_RELAXED);
    }
    
    return (int)sum;
}

int main(int argc, char *argv[]) {
    /* Use argument to prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    printf("Testing reload initialization block...\n");
    
    /* First test: general reload stress */
    int result1 = test_reloads(seed);
    printf("Test 1 result: %d\n", result1);
    
    /* Second test: focus on secondary reloads */
    int result2 = test_secondary_reloads(seed * 2);
    printf("Test 2 result: %d\n", result2);
    
    /* Combine results to ensure all code is live */
    int final_result = result1 + result2;
    printf("Final checksum: %d\n", final_result);
    
    return (final_result == 0) ? 1 : 0;
}
