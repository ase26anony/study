/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to prevent optimization */
    __asm__ volatile ("" : "+r" (x) : : "memory");
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

struct container {
    struct nested n1;
    struct nested n2;
    volatile int v;
    atomic_int atomic;
    char padding[64];
};

/* Global arrays to force complex addressing */
static struct container containers[8][4];
static volatile int volatile_indices[3] = {1, 2, 3};

/* Test function with many register pressures */
__attribute__((noinline, optimize("O1")))
long test_function(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   long l1, long l2, long l3, long l4, long l5,
                   float f1, float f2, double d1, double d2) {
    
    /* Declare many local variables to exhaust registers */
    register int r12_var asm ("r12") = a1;
    register int r13_var asm ("r13") = a2;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    long lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8;
    float fv1, fv2, fv3, fv4;
    double dv1, dv2, dv3;
    
    /* Initialize with arithmetic to create dependencies */
    v1 = barrier(a1 + a2);
    v2 = barrier(a3 * a4);
    v3 = barrier(a5 ^ a6);
    v4 = barrier(a7 | a8);
    v5 = barrier(a9 & a10);
    v6 = barrier(v1 + v2);
    v7 = barrier(v3 - v4);
    v8 = barrier(v5 * v6);
    v9 = barrier(v7 / (v8 ? v8 : 1));
    v10 = barrier(v9 << 2);
    
    v11 = barrier(l1 + a1);
    v12 = barrier(l2 - a2);
    v13 = barrier(l3 * a3);
    v14 = barrier(l4 | a4);
    v15 = barrier(l5 ^ a5);
    v16 = barrier(v11 + v12);
    v17 = barrier(v13 - v14);
    v18 = barrier(v15 * v16);
    v19 = barrier(v17 / (v18 ? v18 : 1));
    v20 = barrier(v19 >> 1);
    
    /* Mixed integer operations */
    lv1 = (long)v1 * (long)v2;
    lv2 = (long)v3 + (long)v4;
    lv3 = (long)v5 - (long)v6;
    lv4 = (long)v7 | (long)v8;
    lv5 = (long)v9 & (long)v10;
    lv6 = lv1 ^ lv2;
    lv7 = lv3 * lv4;
    lv8 = lv5 + lv6 - lv7;
    
    /* Floating point operations to use different register classes */
    fv1 = f1 + f2;
    fv2 = f1 * f2;
    fv3 = fv1 - fv2;
    fv4 = fv3 / (fv2 != 0.0f ? fv2 : 1.0f);
    
    dv1 = d1 + d2;
    dv2 = d1 * d2;
    dv3 = dv1 - dv2;
    
    /* Type punning between int and float */
    union {
        int i;
        float f;
    } punner;
    punner.f = fv4;
    v1 = punner.i;
    
    /* Complex array access with volatile indices - forces addressing mode reloads */
    int idx1 = volatile_indices[0];
    int idx2 = volatile_indices[1];
    int idx3 = volatile_indices[2];
    
    /* SIB-style addressing on x86: base + index*scale + displacement */
    int* volatile ptr = (int*)&containers;
    v2 = ptr[idx1 * 8 + idx2 * 4 + idx3];
    
    /* More complex: nested structure with multi-dimensional array */
    struct container* cptr = &containers[idx1][idx2];
    v3 = cptr->n1.arr[idx3];
    v4 = cptr->n2.arr[(idx1 + idx2) & 3];
    
    /* Atomic operations with memory constraints */
    __atomic_store_n(&cptr->atomic, v3 + v4, __ATOMIC_RELAXED);
    v5 = __atomic_load_n(&cptr->atomic, __ATOMIC_RELAXED);
    
    /* Inline assembly that clobbers many registers */
    /* This forces reloads around the asm block */
    __asm__ volatile (
        "/* Begin clobbering block */\n\t"
        "mov %[in1], %%eax\n\t"
        "mov %[in2], %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %[out]\n\t"
        "/* End clobbering block */"
        : [out] "=r" (v6)
        : [in1] "r" (v1), [in2] "r" (v2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Another asm with memory constraint and complex address */
    int temp;
    __asm__ volatile (
        "movl (%[addr]), %0\n\t"
        "addl $1, %0\n\t"
        "movl %0, (%[addr])"
        : "=r" (temp)
        : [addr] "r" (&cptr->v + idx1 * 16)
        : "memory"
    );
    
    /* Force secondary reload: memory -> specific register -> operation */
    register int forced_reg asm ("ebx") = 0;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "imull %2, %0"
        : "=&r" (forced_reg)
        : "m" (cptr->n1.a), "r" (idx1)
        : "cc"
    );
    
    /* Use forced_reg in another complex operation */
    v7 = forced_reg * v5;
    
    /* More arithmetic to keep all variables live */
    v8 = v1 + v2 + v3 + v4 + v5 + v6 + v7;
    v9 = v8 * v10 - v11 + v12;
    v10 = v13 ^ v14 | v15 & v16;
    
    lv1 = (long)v8 * (long)v9 + (long)v10;
    lv2 = (long)v11 - (long)v12 * (long)v13;
    
    fv1 = (float)v8 + (float)v9 / 2.0f;
    fv2 = (float)v10 * (float)v11 - 3.14f;
    
    /* Final checksum using all variables */
    long checksum = 
        (long)v1 + (long)v2 + (long)v3 + (long)v4 + 
        (long)v5 + (long)v6 + (long)v7 + (long)v8 +
        (long)v9 + (long)v10 + (long)v11 + (long)v12 +
        (long)v13 + (long)v14 + (long)v15 + (long)v16 +
        (long)v17 + (long)v18 + (long)v19 + (long)v20 +
        lv1 + lv2 + lv3 + lv4 + lv5 + lv6 + lv7 + lv8 +
        (long)fv1 + (long)fv2 + (long)fv3 + (long)fv4 +
        (long)dv1 + (long)dv2 + (long)dv3 +
        r12_var + r13_var;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Initialize global arrays */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            containers[i][j].n1.a = i * 100 + j;
            containers[i][j].n2.a = i * 200 + j;
            for (int k = 0; k < 4; k++) {
                containers[i][j].n1.arr[k] = i * 1000 + j * 100 + k;
                containers[i][j].n2.arr[k] = i * 2000 + j * 200 + k;
            }
            containers[i][j].v = i + j;
            __atomic_store_n(&containers[i][j].atomic, i * j, __ATOMIC_RELAXED);
        }
    }
    
    /* Create many live variables to force register pressure */
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
    
    float f1 = (float)argc / 2.0f;
    float f2 = (float)argc / 3.0f;
    double d1 = (double)argc * 1.5;
    double d2 = (double)argc * 2.5;
    
    /* Call test function multiple times with different args */
    long checksum1 = test_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                   l1, l2, l3, l4, l5, f1, f2, d1, d2);
    
    /* Modify some values and call again */
    a1 = barrier(a1 + 1);
    a2 = barrier(a2 * 2);
    volatile_indices[0] = (volatile_indices[0] + 1) & 7;
    
    long checksum2 = test_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                   l1, l2, l3, l4, l5, f1, f2, d1, d2);
    
    printf("Checksum 1: %ld\n", checksum1);
    printf("Checksum 2: %ld\n", checksum2);
    printf("Difference: %ld\n", checksum2 - checksum1);
    
    return (checksum1 != checksum2) ? 0 : 1;
}
