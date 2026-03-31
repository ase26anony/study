/* reload_stress.c - Stress GCC's reload pass to cover reload.cc lines 1381-1399 */

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to make it opaque */
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
volatile int g_volatile = 12345;

/* Test function with many registers and complex addressing */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
long test_reloads(int p1, int p2, int p3, int p4, int p5,
                  int p6, int p7, int p8, int p9, int p10,
                  struct ComplexData* data, int idx1, int idx2, int idx3) {
    
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = p1;  /* Explicit register variable */
    register int r1 asm ("r13") = p2;
    int v1 = p3, v2 = p4, v3 = p5, v4 = p6, v5 = p7;
    int v6 = p8, v7 = p9, v8 = p10;
    long l1 = p1 * 2L, l2 = p2 * 3L, l3 = p3 * 4L, l4 = p4 * 5L;
    float f1 = p5 * 1.5f, f2 = p6 * 2.5f;
    double d1 = p7 * 3.14159, d2 = p8 * 2.71828;
    
    /* Volatile variables to prevent optimization */
    volatile int vol1 = p1;
    volatile long vol2 = p2;
    
    /* Create register pressure with arithmetic chain */
    v1 = barrier(v1 + v2);
    v2 = barrier(v2 + v3);
    v3 = barrier(v3 + v4);
    v4 = barrier(v4 + v5);
    v5 = barrier(v5 + v6);
    v6 = barrier(v6 + v7);
    v7 = barrier(v7 + v8);
    v8 = barrier(v8 + v1);
    
    l1 = barrier(l1 + l2);
    l2 = barrier(l2 + l3);
    l3 = barrier(l3 + l4);
    l4 = barrier(l4 + l1);
    
    /* Mixed integer/float operations */
    f1 = f1 + (float)v1;
    f2 = f2 + (float)v2;
    d1 = d1 + (double)l1;
    d2 = d2 + (double)l2;
    
    /* Complex addressing mode - SIB on x86 */
    /* array[base + index*scale] where scale=4, index may need reload */
    int* base_ptr = &multi_array[0][0][0];
    volatile int scale = 4;  /* Prevent constant propagation */
    int complex_addr_load = base_ptr[idx1 * scale + idx2];
    
    /* More complex: nested array with multiple indices */
    int nested_load = multi_array[idx1 & 7][idx2 & 7][idx3 & 7];
    
    /* Force reload with inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %%eax\n"
        "mov %[val2], %%ebx\n"
        "add %%ebx, %%eax\n"
        "mov %%eax, %[result]\n"
        : [result] "=rm" (v1)  /* Memory or register constraint */
        : [val1] "rm" (v2),    /* Can be memory or register */
          [val2] "rm" (complex_addr_load)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Secondary reload scenario: memory -> specific register -> operation */
    {
        volatile int* volatile_ptr = &data->volatile_member;
        int temp;
        
        /* Force load from volatile memory */
        __atomic_load(volatile_ptr, &temp, __ATOMIC_RELAXED);
        
        /* Use in inline asm with specific register constraint */
        __asm__ volatile (
            "imul %[input], %[output]\n"
            : [output] "+r" (r0)  /* Must be in r12 */
            : [input] "r" (temp)  /* Needs reload to get into register */
            : "cc"
        );
    }
    
    /* Structure element access with complex addressing */
    data->a[(idx1 + idx2) & 7] = v1;
    data->b[idx3 & 3] = l1 + barrier(nested_load);
    
    /* Union for type-punning between int and float */
    union {
        int i;
        float f;
    } punner;
    
    punner.f = f1;
    v2 = punner.i;  /* Forces move between register classes */
    
    /* More arithmetic to use all variables */
    v3 = v1 * v2 + v3 * v4 - v5 / (v6 + 1);
    v4 = v7 | v8 & v1 ^ v2;
    v5 = (v3 << 3) | (v4 >> 2);
    
    l1 = l1 * l2 + l3 * l4;
    l2 = l1 - l2 + l3 - l4;
    
    f1 = f1 * 2.0f + f2;
    d1 = d1 / 2.0 + d2;
    
    /* Another complex addressing mode */
    int* volatile vptr = &data->a[0];
    int idx_sum = idx1 + idx2 + idx3;
    int complex_store = v1 + v2 + v3;
    
    /* This should generate complex addressing */
    vptr[(idx_sum * 2) & 7] = complex_store;
    
    /* Final checksum using all variables */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += l1 + l2 + l3 + l4;
    checksum += (long)f1 + (long)f2;
    checksum += (long)d1 + (long)d2;
    checksum += r0 + r1;
    checksum += complex_addr_load + nested_load;
    checksum += g_volatile;
    
    return barrier(checksum);
}

int main(int argc, char** argv) {
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
    for (int i = 0; i < 2; i++) data.d[i] = i * 3.14159;
    data.volatile_member = 9999;
    
    /* Create many live variables */
    int v1 = argc > 1 ? atoi(argv[1]) : 1;
    int v2 = argc > 2 ? atoi(argv[2]) : 2;
    int v3 = argc > 3 ? atoi(argv[3]) : 3;
    int v4 = argc > 4 ? atoi(argv[4]) : 4;
    int v5 = argc > 5 ? atoi(argv[5]) : 5;
    int v6 = argc > 6 ? atoi(argv[6]) : 6;
    int v7 = argc > 7 ? atoi(argv[7]) : 7;
    int v8 = argc > 8 ? atoi(argv[8]) : 8;
    int v9 = argc > 9 ? atoi(argv[9]) : 9;
    int v10 = argc > 10 ? atoi(argv[10]) : 10;
    
    /* Additional variables for more pressure */
    int v11 = v1 * 2, v12 = v2 * 3, v13 = v3 * 4, v14 = v4 * 5;
    int v15 = v5 * 6, v16 = v6 * 7, v17 = v7 * 8, v18 = v8 * 9;
    long l1 = v1 * 100L, l2 = v2 * 200L, l3 = v3 * 300L, l4 = v4 * 400L;
    
    /* Use volatile indices to prevent optimization */
    volatile int idx1 = v1 & 7;
    volatile int idx2 = v2 & 7;
    volatile int idx3 = v3 & 7;
    
    /* Call test function multiple times with different args */
    long result1 = test_reloads(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,
                               &data, idx1, idx2, idx3);
    
    /* More arithmetic to keep variables live */
    v11 = barrier(v11 + v12);
    v12 = barrier(v12 + v13);
    v13 = barrier(v13 + v14);
    l1 = barrier(l1 + l2);
    l2 = barrier(l2 + l3);
    
    long result2 = test_reloads(v11, v12, v13, v14, v15, v16, v17, v18,
                               v9, v10, &data, idx2, idx3, idx1);
    
    /* Final result depends on all computations */
    long final_result = result1 + result2 + v1 + v2 + v3 + v4 + v5 + 
                       v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + 
                       v14 + v15 + v16 + v17 + v18 + l1 + l2 + l3 + l4;
    
    printf("Result: %ld\n", final_result);
    return (final_result > 0) ? 0 : 1;
}
