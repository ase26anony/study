/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline asm prevents optimization */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Complex structure with mixed types */
struct nested {
    int a;
    long b;
    float c;
    double d;
    int arr[4];
};

/* Global volatile to force memory operations */
volatile int global_index = 0;
volatile long global_offset = 100;

/* Test function with many parameters to force register pressure */
__attribute__((noinline, optimize("O1")))
long test_reloads(int p1, int p2, int p3, int p4, int p5,
                  int p6, int p7, int p8, int p9, int p10,
                  long p11, long p12, long p13, long p14, long p15,
                  float p16, float p17, double p18, double p19) {
    
    /* Many local variables to exhaust registers */
    register int r1 asm ("r12") = p1;
    register int r2 asm ("r13") = p2;
    int v1 = p3, v2 = p4, v3 = p5, v4 = p6, v5 = p7;
    int v6 = p8, v7 = p9, v8 = p10;
    long l1 = p11, l2 = p12, l3 = p13, l4 = p14, l5 = p15;
    float f1 = p16, f2 = p17;
    double d1 = p18, d2 = p19;
    
    /* Additional locals */
    int v9 = barrier(v1), v10 = barrier(v2);
    int v11 = barrier(v3), v12 = barrier(v4);
    long l6 = barrier(l1), l7 = barrier(l2);
    float f3 = f1 * 2.0f, f4 = f2 / 3.0f;
    double d3 = d1 + 1.0, d4 = d2 - 2.0;
    
    /* Multi-dimensional array with volatile index */
    volatile int idx = global_index;
    int md_array[8][8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                md_array[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    /* Complex addressing: SIB-like with all components */
    /* Force secondary reloads for addressing */
    int *base = &md_array[0][0][0];
    int scale = 8 * 8 * sizeof(int);  /* Scale for first dimension */
    long index = idx;
    long offset = global_offset;
    
    /* Access with complex address - may need secondary reload */
    int complex_load = *(int*)((char*)base + index * scale + offset);
    
    /* Inline asm that clobbers many registers */
    /* Forces reloads around asm block */
    __asm__ volatile (
        "/* Begin clobber block */\n\t"
        "movl $0x12345678, %%eax\n\t"
        "movl $0x9ABCDEF0, %%ebx\n\t"
        "movl $0x11111111, %%ecx\n\t"
        "movl $0x22222222, %%edx\n\t"
        "movl $0x33333333, %%esi\n\t"
        "movl $0x44444444, %%edi\n\t"
        "/* End clobber block */"
        : /* no outputs */
        : /* no inputs */
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Use register variables in complex expressions */
    r1 = r1 + v1 * v2 - v3 / (v4 + 1);
    r2 = r2 ^ v5 | v6 & ~v7;
    
    /* Mixed register class operations */
    /* Force moves between integer and FP registers */
    int int_from_float = (int)f1 + (int)f2;
    float float_from_int = (float)v1 + (float)v2;
    
    /* Atomic operations with memory constraints */
    _Atomic int atomic_var = 42;
    int atomic_load = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    __atomic_store_n(&atomic_var, atomic_load + complex_load, __ATOMIC_RELAXED);
    
    /* More complex addressing with structure */
    struct nested nested_array[4];
    for (int i = 0; i < 4; i++) {
        nested_array[i].a = i;
        nested_array[i].b = i * 100L;
        nested_array[i].c = i * 1.5f;
        nested_array[i].d = i * 3.14159;
        for (int j = 0; j < 4; j++) {
            nested_array[i].arr[j] = i * 10 + j;
        }
    }
    
    /* Access nested structure with variable indices */
    volatile int struct_idx = barrier(idx) & 3;
    volatile int arr_idx = barrier(v1) & 3;
    int nested_access = nested_array[struct_idx].arr[arr_idx];
    
    /* Another inline asm with memory constraint */
    /* Forces reload to satisfy constraint */
    int asm_input = v1 + v2 + v3;
    int asm_output;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $0x100, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (asm_output)  /* Memory output - may need reload */
        : "r" (asm_input)    /* Register input - may need reload */
        : "eax"
    );
    
    /* Long dependency chain using all variables */
    long checksum = r1 + r2 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                   v9 + v10 + v11 + v12 +
                   l1 + l2 + l3 + l4 + l5 + l6 + l7 +
                   (long)f1 + (long)f2 + (long)f3 + (long)f4 +
                   (long)d1 + (long)d2 + (long)d3 + (long)d4 +
                   complex_load + atomic_load + nested_access + asm_output +
                   int_from_float + (long)float_from_int;
    
    /* Force spill by using all variables one more time */
    checksum = barrier(checksum) + 
              (r1 * r2) - (v1 | v2) + (v3 & v4) ^ (v5 << 2) +
              (l1 >> 3) * l2 + (l3 - l4) / (l5 + 1) +
              (long)(f1 * f2 * 100.0f) + (long)(d1 / d2 * 1000.0);
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    int i1 = barrier(base + 1);
    int i2 = barrier(base + 2);
    int i3 = barrier(base + 3);
    int i4 = barrier(base + 4);
    int i5 = barrier(base + 5);
    int i6 = barrier(base + 6);
    int i7 = barrier(base + 7);
    int i8 = barrier(base + 8);
    int i9 = barrier(base + 9);
    int i10 = barrier(base + 10);
    
    long l1 = barrier(base + 100);
    long l2 = barrier(base + 200);
    long l3 = barrier(base + 300);
    long l4 = barrier(base + 400);
    long l5 = barrier(base + 500);
    
    float f1 = (float)barrier(base + 20) / 10.0f;
    float f2 = (float)barrier(base + 30) / 10.0f;
    
    double d1 = (double)barrier(base + 40) / 100.0;
    double d2 = (double)barrier(base + 50) / 100.0;
    
    /* Call test function multiple times with different args */
    long result1 = test_reloads(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10,
                               l1, l2, l3, l4, l5,
                               f1, f2, d1, d2);
    
    /* Modify some values and call again */
    i1 = barrier(i1 * 2);
    i2 = barrier(i2 + 7);
    l1 = barrier(l1 >> 1);
    f1 = f1 * 3.14159f;
    
    long result2 = test_reloads(i2, i1, i3, i4, i5, i6, i7, i8, i9, i10,
                               l5, l4, l3, l2, l1,
                               f2, f1, d2, d1);
    
    long final_result = result1 + result2;
    
    /* Print result to prevent dead code elimination */
    printf("Reload stress test result: %ld\n", final_result);
    
    return (final_result != 0) ? 0 : 1;
}
