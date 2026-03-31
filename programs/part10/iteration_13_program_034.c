/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");

/* Force register pressure with many live variables */
__attribute__((noinline))
static long test_reloads(int a1, int a2, int a3, int a4, int a5,
                         int a6, int a7, int a8, int a9, int a10,
                         volatile int* mem_base, int idx1, int idx2) {
    /* Declare many scalar variables to exhaust registers */
    register int r0 asm ("r12") = a1;
    register int r1 asm ("r13") = a2;
    int v1 = a3, v2 = a4, v3 = a5, v4 = a6, v5 = a7;
    int v6 = a8, v7 = a9, v8 = a10;
    long l1 = a1 * 3L, l2 = a2 * 5L, l3 = a3 * 7L;
    long l4 = a4 * 11L, l5 = a5 * 13L, l6 = a6 * 17L;
    float f1 = a7 * 1.5f, f2 = a8 * 2.5f;
    double d1 = a9 * 3.14159, d2 = a10 * 2.71828;
    
    /* Complex addressing with SIB-like computation (triggers secondary reloads) */
    volatile int* volatile_ptr = mem_base;
    int scale = 4;
    int base = idx1 * 8;
    
    /* Force multiple reloads with volatile accesses */
    int temp1 = volatile_ptr[base + idx2 * scale + 1];
    int temp2 = volatile_ptr[base + idx2 * scale + 2];
    
    /* Inline assembly that clobbers many registers */
    asm volatile (
        "mov %[val1], %[out1]\n\t"
        "add %[val2], %[out1]\n\t"
        : [out1] "=r" (v1)
        : [val1] "m" (temp1), [val2] "r" (temp2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* Mixed integer operations creating dependency chain */
    r0 = barrier(r0 + v1);
    r1 = barrier(r1 ^ v2);
    v3 = (v3 * r0) | (v4 & r1);
    v4 = (v4 + v3) ^ (v5 * 3);
    v5 = (v5 - v4) | (v6 >> 2);
    v6 = (v6 * v5) & (v7 + 1);
    v7 = (v7 ^ v6) + (v8 * 7);
    v8 = barrier(v8 | v7);
    
    /* Force spills with many live values across calls */
    l1 = l1 + v1 + barrier(v2);
    l2 = l2 + v3 + barrier(v4);
    l3 = l3 + v5 + barrier(v6);
    l4 = l4 + v7 + barrier(v8);
    l5 = l5 + r0 + barrier(r1);
    l6 = l6 + l1 + barrier(l2);
    
    /* Mixed float/int operations requiring different register classes */
    int if1 = *(int*)&f1;  /* Type-punning */
    int if2 = *(int*)&f2;
    f1 = f1 + (float)if2 * 0.5f;
    f2 = f2 - (float)if1 * 0.25f;
    
    /* Atomic operations with complex addressing */
    _Atomic int atomic_var;
    __atomic_store_n(&atomic_var, v3 + v4, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* More complex addressing with structure */
    struct nested {
        int arr[3][4];
        long vals[2];
    } ns;
    
    volatile int* struct_ptr = &ns.arr[idx1][idx2];
    *struct_ptr = atomic_val + v5;
    
    /* Use register variable in complex constraint */
    int result;
    asm volatile (
        "add %[reg], %[mem], %[out]\n\t"
        : [out] "=r" (result)
        : [reg] "r" (r0), [mem] "m" (*struct_ptr)
        : "cc"
    );
    
    /* Final computation using all variables */
    long checksum = l1 + l2 + l3 + l4 + l5 + l6;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += r0 + r1 + result;
    checksum += (long)(f1 * 100) + (long)(f2 * 100);
    checksum += (long)(d1 * 10) + (long)(d2 * 10);
    checksum += atomic_val + ns.arr[0][0];
    
    return checksum;
}

/* Global memory array for complex addressing */
volatile int global_array[256];

int main(int argc, char** argv) {
    /* Initialize with non-constant values */
    int base_val = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Create many live variables */
    int x1 = base_val + 1, x2 = base_val + 2, x3 = base_val + 3;
    int x4 = base_val + 4, x5 = base_val + 5, x6 = base_val + 6;
    int x7 = base_val + 7, x8 = base_val + 8, x9 = base_val + 9;
    int x10 = base_val + 10, x11 = base_val + 11, x12 = base_val + 12;
    int x13 = base_val + 13, x14 = base_val + 14, x15 = base_val + 15;
    int x16 = base_val + 16, x17 = base_val + 17, x18 = base_val + 18;
    int x19 = base_val + 19, x20 = base_val + 20;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = base_val + i;
    }
    
    /* Call test function multiple times with different indices */
    long total = 0;
    for (int i = 0; i < 3; i++) {
        total += test_reloads(x1 + i, x2 + i, x3 + i, x4 + i, x5 + i,
                             x6 + i, x7 + i, x8 + i, x9 + i, x10 + i,
                             global_array, i, (i * 3) % 8);
    }
    
    /* Use all variables to prevent elimination */
    int dummy = x11 + x12 + x13 + x14 + x15 + x16 + x17 + x18 + x19 + x20;
    total += dummy;
    
    printf("Checksum: %ld\n", total);
    return (int)(total % 256);
}

/* Dummy barrier function definition */
int barrier(int x) {
    volatile int y = x;
    return y ^ 0x55AA55AA;
}
