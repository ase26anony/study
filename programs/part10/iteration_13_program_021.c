/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to prevent optimization */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Volatile memory locations */
volatile int vol_array[256];
volatile long vol_long;

/* Complex structure with nested arrays */
struct nested {
    int a[4];
    struct {
        long b[3];
        float c[2];
    } inner;
    double d;
};

/* Global to prevent optimization */
struct nested global_nested[8];

/* Test function with high register pressure */
__attribute__((noinline, noipa))
int test_reloads(int a, int b, int c, int d, int e, int f, int g, int h,
                 int i, int j, int k, int l, int m, int n, int o, int p) {
    
    /* Many local variables to exhaust registers */
    register int r0 asm ("r12") = a + 1;
    register int r1 asm ("r13") = b + 2;
    int v1 = c + barrier(3);
    int v2 = d + barrier(4);
    int v3 = e + barrier(5);
    int v4 = f + barrier(6);
    int v5 = g + barrier(7);
    int v6 = h + barrier(8);
    int v7 = i + barrier(9);
    int v8 = j + barrier(10);
    int v9 = k + barrier(11);
    int v10 = l + barrier(12);
    int v11 = m + barrier(13);
    int v12 = n + barrier(14);
    int v13 = o + barrier(15);
    int v14 = p + barrier(16);
    int v15 = a * b;
    int v16 = c * d;
    int v17 = e * f;
    int v18 = g * h;
    int v19 = i * j;
    int v20 = k * l;
    
    /* Complex addressing with SIB-like calculation */
    volatile int idx1 = barrier(a) % 8;
    volatile int idx2 = barrier(b) % 4;
    volatile int idx3 = barrier(c) % 3;
    volatile int idx4 = barrier(d) % 2;
    
    /* Force multiple reloads with complex addressing */
    int sum = 0;
    
    /* Access with complex addressing mode */
    sum += global_nested[idx1].a[idx2];
    sum += global_nested[idx2].inner.b[idx3];
    
    /* Mixed integer/float operations */
    {
        float f1 = (float)v1;
        float f2 = (float)v2;
        float f3 = f1 * f2;
        int i3 = (int)f3;
        sum += i3;
        
        /* Force move between register classes */
        union { float f; int i; } pun;
        pun.f = f3;
        sum += pun.i;
    }
    
    /* Inline assembly with many clobbered registers */
    __asm__ volatile (
        "/* Begin clobbering */\n\t"
        "add %[r0], %[v1]\n\t"
        "add %[r1], %[v2]\n\t"
        : [r0] "+r" (r0), [r1] "+r" (r1)
        : [v1] "rm" (v1), [v2] "rm" (v2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* More complex addressing with scale and index */
    for (int scale = 1; scale <= 4; scale++) {
        /* This may require secondary reload on some arches */
        sum += vol_array[(idx1 * scale + idx2) & 255];
    }
    
    /* Atomic operations with complex addresses */
    long atomic_val;
    __atomic_load(&vol_long, &atomic_val, __ATOMIC_RELAXED);
    sum += (int)atomic_val;
    
    /* Another inline asm with memory constraint */
    int temp;
    __asm__ volatile (
        "mov %[temp], %[addr]\n\t"
        "add %[temp], %[offset]\n\t"
        : [temp] "=r" (temp)
        : [addr] "m" (global_nested[0].inner.b[0]),
          [offset] "r" (idx1 * 8 + idx2 * 4)
        : "memory"
    );
    sum += temp;
    
    /* Force spills with long dependency chain */
    v1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    v2 = v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    v3 = v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    v4 = v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
    v5 = v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12;
    v6 = v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13;
    v7 = v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14;
    
    /* Use all variables in final computation */
    sum += r0 + r1 + v1 + v2 + v3 + v4 + v5 + v6 + v7 +
           v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
           v16 + v17 + v18 + v19 + v20;
    
    return sum;
}

/* Secondary test for specific secondary reload patterns */
__attribute__((noinline, noipa))
int test_secondary_reloads(void) {
    volatile int* volatile vol_ptr = &vol_array[0];
    register int reg_var asm ("ebx") = 0;
    int result = 0;
    
    /* This construct often requires secondary reloads */
    for (int i = 0; i < 16; i++) {
        /* Complex address calculation */
        int idx = barrier(i) * 3 + 1;
        
        /* Load from volatile memory with complex address */
        int val = vol_ptr[idx * 2];
        
        /* Inline asm that uses the value with specific constraint */
        __asm__ volatile (
            "addl %[val], %[reg]\n\t"
            : [reg] "+r" (reg_var)
            : [val] "rm" (val)
            : "cc"
        );
        
        result += reg_var;
    }
    
    /* Force floating-point reloads */
    {
        double d1 = 1.0;
        double d2 = 2.0;
        double d3;
        
        /* This may require moves between register classes */
        __asm__ volatile (
            "faddp %%st, %%st(1)\n\t"
            : "=t" (d3)
            : "0" (d1), "u" (d2)
        );
        
        result += (int)d3;
    }
    
    return result;
}

int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        vol_array[i] = i * 3;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            global_nested[i].a[j] = i * 10 + j;
        }
        for (int j = 0; j < 3; j++) {
            global_nested[i].inner.b[j] = i * 20 + j;
        }
        global_nested[i].inner.c[0] = i * 1.5f;
        global_nested[i].inner.c[1] = i * 2.5f;
        global_nested[i].d = i * 3.14159;
    }
    
    vol_long = 0x12345678;
    
    /* Get initial values from argv to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Create many distinct values */
    int vals[16];
    for (int i = 0; i < 16; i++) {
        vals[i] = base + i * 7;
    }
    
    /* Call test function with many arguments */
    int result1 = test_reloads(vals[0], vals[1], vals[2], vals[3],
                               vals[4], vals[5], vals[6], vals[7],
                               vals[8], vals[9], vals[10], vals[11],
                               vals[12], vals[13], vals[14], vals[15]);
    
    int result2 = test_secondary_reloads();
    
    int final_result = result1 + result2;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return final_result == 0 ? 0 : 1;
}
