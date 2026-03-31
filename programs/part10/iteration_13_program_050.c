/* reload_stress.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline asm to prevent optimization */
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

struct container {
    struct nested inner[3];
    volatile int volatile_member;
    atomic_int atomic_member;
};

/* Global arrays to force complex addressing */
static int global_array[256];
static struct container containers[8];

/* Test function with high register pressure */
__attribute__((noinline, noipa))
long test_reloads(int a, int b, int c, int d, int e, int f, int g, int h,
                  int i, int j, int k, int l, int m, int n, int o, int p,
                  int q, int r, int s, int t) {
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a + 1;
    register int r1 asm ("r13") = b + 2;
    register int r2 asm ("r14") = c + 3;
    register int r3 asm ("r15") = d + 4;
    
    int v1 = e + barrier(r0);
    int v2 = f + barrier(r1);
    int v3 = g + barrier(r2);
    int v4 = h + barrier(r3);
    int v5 = i + v1;
    int v6 = j + v2;
    int v7 = k + v3;
    int v8 = l + v4;
    int v9 = m + v5;
    int v10 = n + v6;
    int v11 = o + v7;
    int v12 = p + v8;
    int v13 = q + v9;
    int v14 = r + v10;
    int v15 = s + v11;
    int v16 = t + v12;
    
    /* Force spills with volatile accesses */
    volatile int volatile_index = barrier(v13) & 0xFF;
    volatile int volatile_base = barrier(v14) & 7;
    volatile int volatile_scale = barrier(v15) & 3;
    
    /* Complex addressing mode that may need secondary reload */
    /* array[base + index*scale] - all components used */
    int complex_addr = global_array[
        volatile_base * 32 + 
        volatile_index * (volatile_scale + 1)
    ];
    
    /* Mixed integer/float operations for different register classes */
    float f1 = (float)v1 * 1.5f;
    float f2 = (float)v2 * 2.5f;
    double d1 = (double)v3 * 1.7;
    double d2 = (double)v4 * 2.7;
    
    /* Type punning through union */
    union {
        float f;
        int i;
    } punner;
    punner.f = f1 + f2;
    int punned = punner.i + v5;
    
    /* Inline asm that clobbers many registers */
    /* This forces reloads around the asm block */
    __asm__ volatile (
        "/* Begin clobbering block */\n\t"
        "add %[v6], %[v7], %[out1]\n\t"
        "sub %[v8], %[v9], %[out2]\n\t"
        : [out1] "=r" (v6), [out2] "=r" (v7)
        : [v6] "r" (v6), [v7] "r" (v7), 
          [v8] "r" (v8), [v9] "r" (v9)
        : "r0", "r1", "r2", "r3", "r4", "r5", 
          "r6", "r7", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );
    
    /* More complex addressing with structure access */
    /* containers[base].inner[index].arr[offset] */
    int struct_val = containers[volatile_base & 7]
                     .inner[(volatile_index >> 2) & 2]
                     .arr[volatile_scale & 3];
    
    /* Atomic operations that may need special handling */
    __atomic_store_n(&containers[0].atomic_member, 
                     v10 + v11, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&containers[1].atomic_member, 
                                     __ATOMIC_RELAXED);
    
    /* Another inline asm with memory constraint */
    /* Forces reload for memory operand */
    int mem_operand;
    __asm__ volatile (
        "ldr %0, [%1, %2, lsl #2]\n\t"
        : "=r" (mem_operand)
        : "r" (global_array), "r" (volatile_index)
        : "memory"
    );
    
    /* Use register variable in complex constraint */
    int result;
    __asm__ volatile (
        "mov %[res], %[reg]\n\t"
        "add %[res], %[res], %[mem]\n\t"
        : [res] "=r" (result)
        : [reg] "r" (r0), [mem] "m" (global_array[volatile_index])
        : "cc"
    );
    
    /* Floating point operations to engage FP registers */
    double d3 = d1 * d2 + (double)complex_addr;
    float f3 = f1 - f2 + (float)struct_val;
    
    /* Final computation using all values */
    long checksum = (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                    v11 + v12 + v13 + v14 + v15 + v16 + complex_addr + 
                    punned + atomic_val + mem_operand + result +
                    (long)d3 + (long)f3 + struct_val;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 3; j++) {
            containers[i].inner[j].a = i + j;
            containers[i].inner[j].b = i * j;
            containers[i].inner[j].c = (float)(i) / (j + 1);
            containers[i].inner[j].d = (double)(i * j) / 2.0;
            for (int k = 0; k < 4; k++) {
                containers[i].inner[j].arr[k] = i + j + k;
            }
        }
        containers[i].volatile_member = i * 2;
        __atomic_store_n(&containers[i].atomic_member, i * 3, __ATOMIC_RELAXED);
    }
    
    /* Create many distinct variables with values from argv to prevent constants */
    int vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (argc > i + 1) ? atoi(argv[i + 1]) : (i * 17 + 13);
    }
    
    /* Call test function with all variables */
    long result = test_reloads(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9],
        vars[10], vars[11], vars[12], vars[13], vars[14],
        vars[15], vars[16], vars[17], vars[18], vars[19]
    );
    
    printf("Result: %ld\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return (int)(result & 0x7FFFFFFF);
}
