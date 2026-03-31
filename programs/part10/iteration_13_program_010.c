/* reload_stress.c - Stress GCC's reload pass to cover reload record initialization */
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
    int a;
    long b;
    float c;
    double d;
    int arr[4];
};

/* Global volatile to force memory operations */
volatile int global_index = 0;
volatile long global_offset = 100;

/* Test function with high register pressure */
__attribute__((noinline, optimize("O1")))
long test_reloads(int a1, int a2, int a3, int a4, int a5,
                  int a6, int a7, int a8, int a9, int a10,
                  struct nested* np, int* arr, volatile int* vptr) {
    /* Declare many local variables to exhaust registers */
    register int r1 asm ("r12") = a1;
    register int r2 asm ("r13") = a2;
    int v1 = a3, v2 = a4, v3 = a5, v4 = a6, v5 = a7;
    int v6 = a8, v7 = a9, v8 = a10;
    long l1 = a1 * 2L, l2 = a2 * 3L, l3 = a3 * 4L;
    float f1 = a4 * 1.5f, f2 = a5 * 2.5f;
    double d1 = a6 * 3.14159, d2 = a7 * 2.71828;
    
    /* Complex addressing mode with SIB (for x86) */
    /* array[base + index*scale + offset] */
    int idx = barrier(global_index);
    long base = barrier(global_offset);
    int scale = 4;
    
    /* Force multiple reloads with volatile accesses */
    v1 = *vptr + idx;
    v2 = vptr[base + idx * scale];
    v3 = vptr[barrier(idx) * barrier(scale) + barrier(base)];
    
    /* Inline assembly that clobbers many registers */
    /* This forces the compiler to spill/reload around it */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %[tmp]\n"
        "add %[val2], %[tmp]\n"
        : [tmp] "=r" (v4)
        : [val1] "r" (v1), [val2] "r" (v2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* Mixed register class operations */
    /* Integer to float move requiring different register file */
    int int_for_float = barrier(v3);
    float f_from_int = *(float*)&int_for_float;  /* Type punning */
    
    /* Use explicit register variable in complex expression */
    r1 = barrier(r1 + v4);
    r2 = barrier(r2 * r1);
    
    /* Complex structure access with variable indices */
    np->arr[(idx + 1) & 3] = r1;
    np->arr[(idx + 2) & 3] = r2;
    
    /* Atomic operations with memory ordering */
    int atomic_val = __atomic_load_n(&np->a, __ATOMIC_RELAXED);
    __atomic_store_n(&np->a, atomic_val + v5, __ATOMIC_RELAXED);
    
    /* Multi-dimensional array access */
    int md_arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            md_arr[i][j] = barrier(i * j + idx);
        }
    }
    
    /* More arithmetic to create long dependency chain */
    v5 = v4 + v3 * v2 - v1;
    v6 = v5 * 2 + barrier(v4);
    v7 = v6 / 3 + barrier(v5);
    v8 = v7 ^ v6 | v5;
    
    l1 = l1 + v8 * 100L;
    l2 = l2 + l1 * 2L;
    l3 = l3 + l2 / 3L;
    
    f1 = f1 + f_from_int * 2.0f;
    f2 = f2 + f1 * 3.0f;
    
    d1 = d1 + (double)f2 * 1.5;
    d2 = d2 + d1 * 2.0;
    
    /* Another inline assembly with memory constraint */
    int result;
    __asm__ volatile (
        "# Memory constraint with complex address\n"
        "ldr %[res], [%[addr], %[idx], lsl #2]\n"
        : [res] "=r" (result)
        : [addr] "r" (arr), [idx] "r" (idx)
        : "memory"
    );
    
    /* Use all variables in final computation */
    long checksum = r1 + r2 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8
                  + l1 + l2 + l3
                  + (long)f1 + (long)f2
                  + (long)d1 + (long)d2
                  + result
                  + np->a + np->arr[0] + np->arr[1]
                  + md_arr[idx & 7][0];
    
    return checksum;
}

/* Helper to initialize complex structure */
void init_nested(struct nested* np) {
    np->a = 1;
    np->b = 2L;
    np->c = 3.0f;
    np->d = 4.0;
    for (int i = 0; i < 4; i++) {
        np->arr[i] = i * 10;
    }
}

int main(int argc, char** argv) {
    /* Many scalar variables to create register pressure */
    int var1 = barrier(argc);
    int var2 = barrier(var1 * 2);
    int var3 = barrier(var2 + 1);
    int var4 = barrier(var3 - 1);
    int var5 = barrier(var4 * 3);
    int var6 = barrier(var5 / 2);
    int var7 = barrier(var6 ^ 0xFF);
    int var8 = barrier(var7 | 0xAA);
    int var9 = barrier(var8 & 0x55);
    int var10 = barrier(var9 + 100);
    int var11 = barrier(var10 - 50);
    int var12 = barrier(var11 * 2);
    int var13 = barrier(var12 + var1);
    int var14 = barrier(var13 - var2);
    int var15 = barrier(var14 * var3);
    int var16 = barrier(var15 / 4);
    int var17 = barrier(var16 | 0x1234);
    int var18 = barrier(var17 ^ 0x5678);
    int var19 = barrier(var18 + 999);
    int var20 = barrier(var19 - 888);
    
    /* Complex structure */
    struct nested nested_obj;
    init_nested(&nested_obj);
    
    /* Arrays for complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = barrier(i * i);
    }
    
    /* Volatile pointer */
    volatile int volatile_data[128];
    for (int i = 0; i < 128; i++) {
        volatile_data[i] = barrier(i * 3);
    }
    
    /* Call test function multiple times with different args */
    long total = 0;
    for (int i = 0; i < 10; i++) {
        global_index = barrier(i);
        total += test_reloads(
            var1 + i, var2, var3, var4, var5,
            var6, var7, var8, var9, var10,
            &nested_obj, array, volatile_data
        );
        
        /* Modify some variables to prevent optimization */
        var1 = barrier(var1 + 1);
        var2 = barrier(var2 * 2);
        var3 = barrier(var3 - 1);
    }
    
    printf("Checksum: %ld\n", total);
    return (int)(total % 256);
}
