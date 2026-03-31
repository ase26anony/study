/* reload_stress_test.c
 * A program designed to stress GCC's reload pass and trigger initialization
 * of reload records, particularly those needing secondary reloads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Use inline asm to make it opaque */
    __asm__ volatile ("" : "+r"(x));
    return x;
}

/* Force register pressure with many live variables */
__attribute__((noinline))
static long test_reloads(int a, int b, int c, int d, int e,
                         int f, int g, int h, int i, int j,
                         int k, int l, int m, int n, int o,
                         int p, int q, int r, int s, int t) {
    /* Declare many local variables to exhaust registers */
    volatile int v1 = barrier(a);
    volatile int v2 = barrier(b);
    volatile int v3 = barrier(c);
    volatile int v4 = barrier(d);
    volatile int v5 = barrier(e);
    volatile int v6 = barrier(f);
    volatile int v7 = barrier(g);
    volatile int v8 = barrier(h);
    volatile int v9 = barrier(i);
    volatile int v10 = barrier(j);
    volatile int v11 = barrier(k);
    volatile int v12 = barrier(l);
    volatile int v13 = barrier(m);
    volatile int v14 = barrier(n);
    volatile int v15 = barrier(o);
    volatile int v16 = barrier(p);
    volatile int v17 = barrier(q);
    volatile int v18 = barrier(r);
    volatile int v19 = barrier(s);
    volatile int v20 = barrier(t);
    
    /* Complex array with multi-dimensional access */
    int array[10][10][10];
    volatile int idx1 = barrier(a) % 10;
    volatile int idx2 = barrier(b) % 10;
    volatile int idx3 = barrier(c) % 10;
    
    /* Initialize array with values */
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                array[x][y][z] = x * 100 + y * 10 + z;
            }
        }
    }
    
    /* Complex addressing mode: array[idx1][idx2][idx3] with scaling */
    /* This may require secondary reloads on some architectures */
    int result = 0;
    
    /* Force SIB addressing on x86 or similar complex addressing on RISC */
    for (int scale = 1; scale <= 4; scale *= 2) {
        /* Complex address calculation that may need reloads */
        result += array[idx1][idx2][idx3] * scale;
        
        /* More complex: array with variable index and base */
        volatile int base = barrier(scale);
        result += array[base % 10][(base + 1) % 10][(base + 2) % 10];
    }
    
    /* Inline assembly that clobbers many registers */
    /* This forces the compiler to spill/reload around the asm */
    __asm__ volatile (
        "# Clobber many registers to force reloads\n"
        "mov %0, %0\n"  /* Use the input */
        :
        : "r" (result)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    /* Mix integer and floating point operations */
    /* Forces moves between different register classes */
    {
        union {
            int i;
            float f;
        } pun;
        
        pun.i = result;
        float fval = pun.f * 1.5f;
        pun.f = fval;
        result = pun.i;
    }
    
    /* Use explicit register variables to create conflicts */
    register int reg_var1 asm ("r12") = v1;
    register int reg_var2 asm ("r13") = v2;
    
    /* Complex expression using register variables */
    result += reg_var1 * reg_var2;
    
    /* Inline asm with memory constraint and complex address */
    {
        volatile int mem_location = 0x12345678;
        int temp;
        
        /* This may require secondary reload for the memory address */
        __asm__ volatile (
            "ldr %0, [%1]\n"
            : "=r" (temp)
            : "r" (&mem_location)
            : "memory"
        );
        
        result ^= temp;
    }
    
    /* Atomic operations that may have specific reload requirements */
    {
        _Atomic int atomic_var = ATOMIC_VAR_INIT(42);
        int old_val = __atomic_exchange_n(&atomic_var, result, __ATOMIC_RELAXED);
        result += old_val;
    }
    
    /* More arithmetic to create long dependency chain */
    result = ((result * v3) / (v4 + 1)) | (v5 << 3);
    result ^= v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result += v11 * v12 - v13 / (v14 | 1);
    result |= v15 & v16 & v17;
    result = (result << v18) | (result >> (32 - v18));
    result += v19 * 0x9e3779b9;
    result ^= v20;
    
    /* Another inline asm that uses specific output registers */
    {
        int out1, out2;
        __asm__ volatile (
            "# Force output to specific registers\n"
            "mov %0, %2\n"
            "mov %1, %3\n"
            : "=&r" (out1), "=&r" (out2)
            : "r" (result), "r" (result ^ 0x55555555)
            : "cc"
        );
        result = out1 + out2;
    }
    
    /* Access complex structure with volatile members */
    {
        struct complex_struct {
            volatile int a;
            int b;
            volatile int c[5];
            int d;
        } cs;
        
        cs.a = v1;
        cs.b = v2;
        for (int i = 0; i < 5; i++) {
            cs.c[i] = v3 + i;
        }
        cs.d = v4;
        
        /* Complex addressing within structure */
        result += cs.a + cs.b + cs.c[idx1 % 5] + cs.d;
    }
    
    /* Force spill by using all variables in one expression */
    result = barrier(
        result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20
    );
    
    return result;
}

/* Another test function focusing on memory addressing modes */
__attribute__((noinline))
static int test_complex_addressing(int *base, int index, int scale) {
    /* Complex addressing that may need secondary reload */
    int result = 0;
    
    /* Force SIB-like addressing: base + index * scale + offset */
    for (int offset = 0; offset < 100; offset += 10) {
        /* This address calculation may need reloads */
        result += base[index * scale + offset];
        
        /* More complex: nested addressing */
        volatile int idx2 = barrier(index);
        result += base[(idx2 * 2) + (offset / 2)];
    }
    
    /* Use the result in inline asm with memory constraint */
    __asm__ volatile (
        "# Memory constraint with complex address\n"
        "add %0, %0, #1\n"
        : "+m" (*base)
        :
        : "cc"
    );
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = barrier(argc + i * 0x12345);
    }
    
    /* Call test function with many arguments to force register passing */
    long result = test_reloads(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9],
        vars[10], vars[11], vars[12], vars[13], vars[14],
        vars[15], vars[16], vars[17], vars[18], vars[19]
    );
    
    /* Test complex addressing */
    int array[1000];
    for (int i = 0; i < 1000; i++) {
        array[i] = barrier(i * 0x56789);
    }
    
    int addr_result = test_complex_addressing(array, argc % 50, 4);
    
    /* Combine results */
    result += addr_result;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld (argc=%d)\n", result, argc);
    
    return (result > 0) ? 0 : 1;
}
