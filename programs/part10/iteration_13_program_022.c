/* reload_stress_test.c
 * Stress test for GCC's reload pass to cover reload record initialization.
 * Compile with: gcc -O1 -fschedule-insns -fno-omit-frame-pointer -m32 -march=i686 -fno-pic -fno-optimize-sibling-calls -fno-crossjumping reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __attribute__((noinline));
int barrier(int x) {
    volatile int v = x;
    return v + 1;
}

/* Complex structure to force complex addressing */
struct nested {
    int a[4];
    long b[3];
    struct {
        int x;
        int y;
    } inner[2];
};

/* Multi-dimensional array with volatile indices */
volatile int idx1 = 1, idx2 = 2, idx3 = 3;

/* Register variables to force specific register allocation */
register int reg_var1 asm ("ebx");
register int reg_var2 asm ("esi");
register int reg_var3 asm ("edi");

/* Test function with many parameters and complex operations */
__attribute__((noinline, noipa))
int test_reloads(int p1, int p2, int p3, int p4, int p5,
                 int p6, int p7, int p8, int p9, int p10,
                 int p11, int p12, int p13, int p14, int p15,
                 long p16, long p17, long p18, long p19, long p20) {
    
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3;
    
    /* Initialize with parameters to create dependencies */
    v1 = p1 + p2; v2 = p3 - p4; v3 = p5 * p6; v4 = p7 ^ p8;
    v5 = p9 | p10; v6 = p11 & p12; v7 = p13 << 2; v8 = p14 >> 1;
    v9 = p15 + 42; v10 = barrier(p1);
    
    v11 = v1 + v2; v12 = v3 - v4; v13 = v5 * v6; v14 = v7 ^ v8;
    v15 = v9 | v10; v16 = v11 & v12; v17 = v13 << 1; v18 = v14 >> 2;
    v19 = v15 + v16; v20 = v17 - v18;
    
    l1 = p16 + p17; l2 = p18 - p19; l3 = p20 * 3;
    l4 = l1 ^ l2; l5 = l3 & 0xFF; l6 = l4 << 3; l7 = l5 >> 2;
    l8 = l6 + l7; l9 = l8 * 2; l10 = barrier(l9);
    
    /* Mixed integer/float operations to force moves between register classes */
    f1 = (float)v1 + (float)v2;
    f2 = (float)v3 * (float)v4;
    f3 = f1 - f2;
    f4 = (float)l1 / 2.0f;
    f5 = f3 + f4;
    
    d1 = (double)l2 + (double)l3;
    d2 = (double)v5 * 1.5;
    d3 = d1 - d2;
    
    /* Complex array access with SIB addressing (for x86) */
    int array[100][10];
    volatile int *volatile ptr = (volatile int*)array;
    
    /* Force complex addressing: array[base + index*scale] */
    for (int i = 0; i < 10; i++) {
        /* This should generate SIB addressing on x86 */
        int index = barrier(i);
        array[index][i*2] = v1 + i;
        array[i*3][index] = v2 - i;
        
        /* More complex addressing with multiple components */
        ptr[i*4 + index] = v3 * i;
    }
    
    /* Complex structure access */
    struct nested nested_array[5];
    for (int i = 0; i < 5; i++) {
        nested_array[i].a[i] = v4 + i;
        nested_array[i].b[i%3] = l4 + i;
        nested_array[i].inner[i%2].x = v5 * i;
        nested_array[i].inner[i%2].y = v6 + i;
    }
    
    /* Inline assembly that clobbers many registers */
    int result;
    asm volatile (
        "movl %[val1], %%eax\n\t"
        "addl %[val2], %%eax\n\t"
        "movl %%eax, %[res]\n\t"
        : [res] "=r" (result)
        : [val1] "r" (v7), [val2] "r" (v8)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* More inline assembly with memory constraints */
    int temp;
    asm volatile (
        "movl (%[addr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[addr])\n\t"
        : 
        : [addr] "r" (&v9)
        : "eax", "memory"
    );
    
    /* Use register variables in complex expressions */
    reg_var1 = v10;
    reg_var2 = v11;
    reg_var3 = v12;
    
    asm volatile (
        "addl %%ebx, %%esi\n\t"
        "addl %%edi, %%esi\n\t"
        "movl %%esi, %0\n\t"
        : "=r" (temp)
        : 
        : "ebx", "esi", "edi"
    );
    
    /* Atomic operations to force specific reload patterns */
    __atomic_store_n(&v13, v14 + temp, __ATOMIC_RELAXED);
    int loaded = __atomic_load_n(&v15, __ATOMIC_RELAXED);
    
    /* Complex expression using all variables to create register pressure */
    int checksum = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5 +
        (int)l6 + (int)l7 + (int)l8 + (int)l9 + (int)l10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)d1 + (int)d2 + (int)d3 +
        result + temp + loaded + reg_var1 + reg_var2 + reg_var3;
    
    /* Access array with volatile indices to prevent optimization */
    checksum += array[idx1][idx2];
    checksum += nested_array[idx3].inner[0].x;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Create many live variables */
    int a1 = base + 1, a2 = base + 2, a3 = base + 3, a4 = base + 4, a5 = base + 5;
    int a6 = base + 6, a7 = base + 7, a8 = base + 8, a9 = base + 9, a10 = base + 10;
    int a11 = base + 11, a12 = base + 12, a13 = base + 13, a14 = base + 14, a15 = base + 15;
    long l1 = base + 100, l2 = base + 200, l3 = base + 300, l4 = base + 400, l5 = base + 500;
    long l6 = base + 600, l7 = base + 700, l8 = base + 800, l9 = base + 900, l10 = base + 1000;
    
    /* Call test function multiple times with different arguments */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += test_reloads(
            a1 + i, a2 + i, a3 + i, a4 + i, a5 + i,
            a6 + i, a7 + i, a8 + i, a9 + i, a10 + i,
            a11 + i, a12 + i, a13 + i, a14 + i, a15 + i,
            l1 + i, l2 + i, l3 + i, l4 + i, l5 + i
        );
        
        /* Modify variables to prevent optimization */
        a1 = barrier(a1);
        a2 = barrier(a2);
        l1 = barrier(l1);
        l2 = barrier(l2);
    }
    
    printf("Checksum: %d\n", sum);
    return 0;
}
