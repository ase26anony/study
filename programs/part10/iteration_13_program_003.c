/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Implementation not visible to compiler during compilation */
    return x;
}

/* Force register pressure with many live variables */
__attribute__((noinline))
int test_reloads(int seed) {
    /* Declare many scalar variables to exhaust registers */
    register int r0 asm ("r12") = seed + 1;
    register int r1 asm ("r13") = seed + 2;
    volatile int v0 = seed + 3;
    volatile int v1 = seed + 4;
    int a = seed + 5, b = seed + 6, c = seed + 7, d = seed + 8;
    int e = seed + 9, f = seed + 10, g = seed + 11, h = seed + 12;
    int i = seed + 13, j = seed + 14, k = seed + 15, l = seed + 16;
    int m = seed + 17, n = seed + 18, o = seed + 19, p = seed + 20;
    int q = seed + 21, r = seed + 22, s = seed + 23, t = seed + 24;
    
    /* Complex multi-dimensional array with volatile indices */
    volatile int idx1 = seed % 5;
    volatile int idx2 = seed % 3;
    int array[5][4][3];
    
    /* Initialize array with values */
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 4; y++) {
            for (int z = 0; z < 3; z++) {
                array[x][y][z] = seed + x * 100 + y * 10 + z;
            }
        }
    }
    
    /* Force SIB/addressing mode reloads with complex array access */
    /* This should trigger secondary reloads on many architectures */
    a += array[idx1][idx2][0];
    b += array[idx1 + 1][idx2][1];
    c += array[idx1][idx2 + 1][2];
    
    /* Inline assembly that clobbers many registers */
    /* Forces reloads around the asm block */
    asm volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %[val2]\n\t"
        "add %[val3], %[val4]\n\t"
        : [val1] "+r" (r0), [val2] "+r" (r1)
        : [val3] "r" (a), [val4] "r" (b)
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
    
    /* Mixed integer/floating point operations */
    /* Forces moves between different register classes */
    float f1 = (float)a / 2.0f;
    float f2 = (float)b / 3.0f;
    union {
        float f;
        int i;
    } pun;
    pun.f = f1;
    d += pun.i;  /* Type punning forces register class change */
    
    /* More arithmetic to create long dependency chain */
    e = a + b + c + d;
    f = e * 2 - d;
    g = f / 3 + c;
    h = g << 2;
    i = h >> 1;
    j = i ^ e;
    k = j | f;
    l = k & g;
    m = l + h;
    n = m - i;
    o = n * j;
    p = o / (k + 1);
    q = p % (l + 1);
    r = q ^ m;
    s = r | n;
    t = s & o;
    
    /* Atomic operations with memory ordering */
    /* Generates specific reload patterns */
    __atomic_store(&v0, &t, __ATOMIC_RELAXED);
    int loaded;
    __atomic_load(&v1, &loaded, __ATOMIC_RELAXED);
    
    /* Complex addressing in inline assembly output */
    /* Should trigger secondary reload initialization */
    int *ptr = &array[idx1][idx2][0];
    int result;
    asm volatile (
        "# Memory operand with complex addressing\n"
        "mov %[src], %[dst]\n\t"
        : [dst] "=r" (result)
        : [src] "m" (*ptr)
        : "memory"
    );
    
    /* Use all variables in final computation */
    int checksum = r0 + r1 + v0 + v1 + a + b + c + d + e + f + g + h + 
                   i + j + k + l + m + n + o + p + q + r + s + t + 
                   result + loaded + barrier(seed);
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (checksum));
    
    return checksum;
}

/* Another function to create more reload contexts */
__attribute__((noinline))
int more_reloads(int x, int y, int z) {
    volatile long vl1 = x, vl2 = y, vl3 = z;
    
    /* Structure with nested access */
    struct nested {
        int a;
        struct {
            int b[4];
            int c;
        } inner;
        int d[3][2];
    } s;
    
    s.a = x;
    s.inner.c = y;
    for (int i = 0; i < 4; i++) s.inner.b[i] = x + i;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++)
            s.d[i][j] = x + i * 10 + j;
    
    /* Complex structure addressing */
    volatile int idx = z % 3;
    int val1 = s.inner.b[idx];
    int val2 = s.d[idx][z % 2];
    
    /* Force spill with many live values */
    int t1 = val1 + vl1;
    int t2 = val2 + vl2;
    int t3 = t1 * t2;
    int t4 = t3 / (vl3 + 1);
    int t5 = t4 << 2;
    int t6 = t5 >> 1;
    int t7 = t6 ^ t1;
    int t8 = t7 | t2;
    int t9 = t8 & t3;
    int t10 = t9 + t4;
    
    /* Vector-like operations (if supported) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {t1, t2, t3, t4};
    v4si v2 = {t5, t6, t7, t8};
    v4si v3 = v1 + v2;
    
    /* Extract from vector - forces move between register classes */
    int vextract;
    asm volatile (
        "# Extract element from vector\n"
        "movd %[vec], %[out]\n\t"
        : [out] "=r" (vextract)
        : [vec] "x" (v3)
    );
    
    return t10 + vextract + barrier(x + y + z);
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Call test functions multiple times with different seeds */
    int sum = 0;
    for (int iter = 0; iter < 3; iter++) {
        int result1 = test_reloads(seed + iter * 100);
        int result2 = more_reloads(seed + iter, seed + iter * 2, seed + iter * 3);
        sum += result1 + result2;
        
        /* Prevent loop optimization */
        asm volatile ("" : : "r" (result1), "r" (result2));
    }
    
    printf("Reload stress test checksum: %d\n", sum);
    return (sum != 0) ? 0 : 1;
}
