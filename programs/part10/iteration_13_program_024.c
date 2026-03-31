/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
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
    struct nested n1;
    struct nested n2;
    volatile int v;
    atomic_int atomic;
};

/* Global arrays to force complex addressing */
static int global_array[256];
static struct container containers[16];

/* Test function with many registers and complex operations */
__attribute__((noinline, optimize("O1")))
long test_reloads(int a, long b, int c, long d, int e, long f,
                  int g, long h, int i, long j, int k, long l,
                  int m, long n, int o, long p, int q, long r,
                  int s, long t) {
    
    /* Declare many local variables to increase register pressure */
    register int r1 asm ("r12") = a + 1;
    register int r2 asm ("r13") = b + 2;
    volatile int v1 = c;
    volatile int v2 = d;
    float f1 = (float)a;
    double d1 = (double)b;
    long long ll1 = (long long)a * b;
    
    /* Force spills with many live variables */
    int var1 = barrier(a);
    int var2 = barrier(b);
    int var3 = barrier(c);
    int var4 = barrier(d);
    int var5 = barrier(e);
    int var6 = barrier(f);
    int var7 = barrier(g);
    int var8 = barrier(h);
    int var9 = barrier(i);
    int var10 = barrier(j);
    int var11 = barrier(k);
    int var12 = barrier(l);
    int var13 = barrier(m);
    int var14 = barrier(n);
    int var15 = barrier(o);
    int var16 = barrier(p);
    int var17 = barrier(q);
    int var18 = barrier(r);
    int var19 = barrier(s);
    int var20 = barrier(t);
    
    /* Complex arithmetic creating dependency chain */
    var1 = var1 * var2 + var3;
    var2 = var2 * var3 + var4;
    var3 = var3 * var4 + var5;
    var4 = var4 * var5 + var6;
    var5 = var5 * var6 + var7;
    var6 = var6 * var7 + var8;
    var7 = var7 * var8 + var9;
    var8 = var8 * var9 + var10;
    var9 = var9 * var10 + var11;
    var10 = var10 * var11 + var12;
    var11 = var11 * var12 + var13;
    var12 = var12 * var13 + var14;
    var13 = var13 * var14 + var15;
    var14 = var14 * var15 + var16;
    var15 = var15 * var16 + var17;
    var16 = var16 * var17 + var18;
    var17 = var17 * var18 + var19;
    var18 = var18 * var19 + var20;
    var19 = var19 * var20 + var1;
    var20 = var20 * var1 + var2;
    
    /* Inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "mov %[v1], %%eax\n"
        "mov %[v2], %%ebx\n"
        "add %%ebx, %%eax\n"
        "mov %%eax, %[result]\n"
        : [result] "=r" (var1)
        : [v1] "m" (v1), [v2] "m" (v2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Complex addressing modes - SIB addressing on x86 */
    volatile int idx1 = var1 % 16;
    volatile int idx2 = var2 % 4;
    volatile int idx3 = var3 % 256;
    
    /* Force secondary reloads with complex memory addressing */
    int temp1 = containers[idx1].n1.arr[idx2] + 
                global_array[idx3 + idx1 * 4];
    
    /* More complex addressing with scaling */
    int temp2 = global_array[idx1 * 8 + idx2 * 2 + 16];
    
    /* Mixed register class operations */
    f1 = f1 + (float)temp1;
    d1 = d1 + (double)temp2;
    
    /* Type punning through union */
    union {
        float f;
        int i;
    } pun;
    pun.f = f1;
    var1 = var1 ^ pun.i;
    
    /* Atomic operations that need special handling */
    __atomic_store_n(&containers[idx1].atomic, var1, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&containers[idx2].atomic, __ATOMIC_RELAXED);
    
    /* Another inline asm with complex constraints */
    long final_result;
    __asm__ volatile (
        "# More complex assembly\n"
        "imul %[a], %[b]\n"
        "add %[c], %[b]\n"
        : [b] "+r" (ll1)
        : [a] "r" (atomic_val), [c] "m" (containers[idx1].v)
        : "cc", "memory"
    );
    
    /* Force floating point reloads */
    double d2 = d1 * 2.0;
    float f2 = f1 * 3.0f;
    
    /* Use all variables in final computation */
    final_result = (long)var1 + (long)var2 + (long)var3 + (long)var4 +
                   (long)var5 + (long)var6 + (long)var7 + (long)var8 +
                   (long)var9 + (long)var10 + (long)var11 + (long)var12 +
                   (long)var13 + (long)var14 + (long)var15 + (long)var16 +
                   (long)var17 + (long)var18 + (long)var19 + (long)var20 +
                   (long)temp1 + (long)temp2 + (long)atomic_val +
                   (long)ll1 + (long)d2 + (long)f2 + r1 + r2;
    
    return final_result;
}

int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        containers[i].n1.a = i;
        containers[i].n1.b = i * 2L;
        containers[i].n1.c = i * 3.0f;
        containers[i].n1.d = i * 4.0;
        for (int j = 0; j < 4; j++) {
            containers[i].n1.arr[j] = i * 10 + j;
        }
        containers[i].v = i * 100;
        __atomic_store_n(&containers[i].atomic, i * 50, __ATOMIC_RELAXED);
    }
    
    /* Create many distinct variables to force register pressure */
    int a1 = barrier(argc + 1);
    long b1 = barrier(argc + 2);
    int c1 = barrier(argc + 3);
    long d1 = barrier(argc + 4);
    int e1 = barrier(argc + 5);
    long f1 = barrier(argc + 6);
    int g1 = barrier(argc + 7);
    long h1 = barrier(argc + 8);
    int i1 = barrier(argc + 9);
    long j1 = barrier(argc + 10);
    int k1 = barrier(argc + 11);
    long l1 = barrier(argc + 12);
    int m1 = barrier(argc + 13);
    long n1 = barrier(argc + 14);
    int o1 = barrier(argc + 15);
    long p1 = barrier(argc + 16);
    int q1 = barrier(argc + 17);
    long r1 = barrier(argc + 18);
    int s1 = barrier(argc + 19);
    long t1 = barrier(argc + 20);
    
    /* Call test function multiple times with different args */
    long result1 = test_reloads(a1, b1, c1, d1, e1, f1, g1, h1, i1, j1,
                               k1, l1, m1, n1, o1, p1, q1, r1, s1, t1);
    
    /* Modify variables and call again */
    a1 = barrier(a1 * 2);
    b1 = barrier(b1 * 3);
    long result2 = test_reloads(a1, b1, c1, d1, e1, f1, g1, h1, i1, j1,
                               k1, l1, m1, n1, o1, p1, q1, r1, s1, t1);
    
    printf("Result1: %ld\n", result1);
    printf("Result2: %ld\n", result2);
    printf("Checksum: %ld\n", result1 + result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}
