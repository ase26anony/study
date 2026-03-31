/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to prevent optimization */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Complex structure with mixed types */
struct nested {
    int a[3];
    long b[2];
    float c;
    double d;
};

union type_pun {
    int i;
    float f;
    unsigned u;
    void *p;
};

/* Global volatile to force memory operations */
volatile int global_index = 0;
volatile long global_offset = 100;

/* Test function with many register pressures */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
long test_reloads(int a1, int a2, int a3, int a4, int a5,
                  int a6, int a7, int a8, int a9, int a10,
                  long l1, long l2, long l3, long l4, long l5,
                  float f1, float f2, double d1, double d2) {
    
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a1;
    register int r1 asm ("r13") = a2;
    register int r2 asm ("r14") = a3;
    register int r3 asm ("r15") = a4;
    
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    long lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8;
    float fv1, fv2, fv3, fv4;
    double dv1, dv2, dv3;
    
    /* Initialize with complex expressions */
    v1 = barrier(a1 + a2);
    v2 = barrier(a3 * a4);
    v3 = barrier(a5 ^ a6);
    v4 = barrier(a7 | a8);
    v5 = barrier(a9 & a10);
    
    /* Force register pressure with long chain */
    v6 = v1 + v2 + r0;
    v7 = v3 * v4 - r1;
    v8 = (v5 << 2) | r2;
    v9 = barrier(v6 ^ v7);
    v10 = barrier(v8 + v9);
    
    /* More variables to increase pressure */
    v11 = a1 * 3 + a2 * 7;
    v12 = a3 / 2 + a4 * 5;
    v13 = barrier(v11 + v12);
    v14 = barrier(a5 * a6 - a7);
    v15 = barrier(a8 | a9 & a10);
    v16 = v13 + v14 + v15;
    v17 = barrier(v16 * 3);
    v18 = barrier(v17 >> 2);
    v19 = v18 + r3;
    v20 = barrier(v19 * 2);
    
    /* Long variables */
    lv1 = l1 + l2;
    lv2 = l3 * l4;
    lv3 = barrier(lv1 | lv2);
    lv4 = l5 + global_offset;
    lv5 = barrier(lv3 ^ lv4);
    lv6 = lv5 * 2 + 1;
    lv7 = barrier(lv6 - lv4);
    lv8 = lv7 >> 3;
    
    /* Floating point to engage different register classes */
    fv1 = f1 * 2.0f;
    fv2 = f2 + 1.5f;
    fv3 = barrier(fv1 - fv2);
    fv4 = fv3 * 3.14f;
    
    dv1 = d1 * 1.618;
    dv2 = d2 + 2.718;
    dv3 = barrier(dv1 / dv2);
    
    /* Complex array access with volatile indices */
    volatile int idx1 = global_index;
    volatile int idx2 = barrier(idx1 + 1);
    
    /* Multi-dimensional array with complex addressing */
    int md_array[5][7][3];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 7; j++) {
            for (int k = 0; k < 3; k++) {
                /* Complex addressing: base + scale*index + offset */
                md_array[i][j][k] = i * 100 + j * 10 + k + idx1;
            }
        }
    }
    
    /* Access with SIB-like addressing (for x86) */
    int *base = &md_array[0][0][0];
    int scale = 21; /* 7*3 */
    int result1 = base[idx1 * scale + idx2 * 3];
    int result2 = base[idx2 * scale + idx1 * 3];
    
    /* Inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %%eax\n"
        "mov %[val2], %%ebx\n"
        "add %%ebx, %%eax\n"
        "mov %%eax, %[out]\n"
        : [out] "=r" (v20)
        : [val1] "m" (result1), [val2] "r" (result2)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* More inline assembly with memory constraints */
    int temp;
    __asm__ volatile (
        "movl %[addr], %[temp]\n"
        "addl $1, %[temp]\n"
        : [temp] "=r" (temp)
        : [addr] "m" (md_array[idx1][idx2][0])
        : "memory"
    );
    
    /* Type punning between int and float */
    union type_pun pun1, pun2;
    pun1.i = v20;
    pun2.f = fv4;
    
    /* Atomic operations that force memory reloads */
    _Atomic int atomic_var = 0;
    __atomic_store_n(&atomic_var, v20, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* Complex structure access */
    struct nested complex_struct[4];
    for (int i = 0; i < 4; i++) {
        complex_struct[i].a[0] = i * 10 + atomic_val;
        complex_struct[i].b[1] = i * 100 + lv8;
        complex_struct[i].c = fv4 * i;
        complex_struct[i].d = dv3 * i;
    }
    
    /* Access with double register addressing simulation */
    long struct_base = (long)&complex_struct[0];
    long offset = idx1 * sizeof(struct nested);
    struct nested *ptr = (struct nested *)(struct_base + offset);
    
    /* Final computation using all variables */
    long checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += v6 + v7 + v8 + v9 + v10;
    checksum += v11 + v12 + v13 + v14 + v15;
    checksum += v16 + v17 + v18 + v19 + v20;
    checksum += lv1 + lv2 + lv3 + lv4;
    checksum += lv5 + lv6 + lv7 + lv8;
    checksum += (long)(fv1 * 100) + (long)(fv2 * 100);
    checksum += (long)(fv3 * 100) + (long)(fv4 * 100);
    checksum += (long)(dv1 * 100) + (long)(dv2 * 100) + (long)(dv3 * 100);
    checksum += result1 + result2 + temp + atomic_val;
    checksum += ptr->a[0] + ptr->b[1];
    checksum += (long)(ptr->c * 100) + (long)(ptr->d * 100);
    checksum += pun1.i + (long)pun2.f;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    int a1 = barrier(base + 1);
    int a2 = barrier(base + 2);
    int a3 = barrier(base + 3);
    int a4 = barrier(base + 4);
    int a5 = barrier(base + 5);
    int a6 = barrier(base + 6);
    int a7 = barrier(base + 7);
    int a8 = barrier(base + 8);
    int a9 = barrier(base + 9);
    int a10 = barrier(base + 10);
    
    long l1 = barrier(base * 100L + 1);
    long l2 = barrier(base * 100L + 2);
    long l3 = barrier(base * 100L + 3);
    long l4 = barrier(base * 100L + 4);
    long l5 = barrier(base * 100L + 5);
    
    float f1 = barrier(base) * 1.1f;
    float f2 = barrier(base) * 2.2f;
    double d1 = barrier(base) * 3.14159;
    double d2 = barrier(base) * 2.71828;
    
    /* Call test function multiple times with different args */
    long total = 0;
    for (int i = 0; i < 3; i++) {
        global_index = i;
        long result = test_reloads(a1 + i, a2, a3, a4, a5,
                                  a6, a7, a8, a9, a10,
                                  l1, l2, l3, l4, l5,
                                  f1, f2, d1, d2);
        total += result;
        printf("Iteration %d: checksum = %ld\n", i, result);
    }
    
    printf("Total checksum: %ld\n", total);
    return (total > 0) ? 0 : 1;
}
