/* test_early_remat.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Vector type to engage different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test function 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int barrier = seed;
    
    /* Create many independent computations */
    int a = barrier * 3 + 7;
    int b = barrier / 2 - 5;
    int c = barrier & 0xFF;
    int d = barrier | 0xAA;
    int e = barrier ^ 0x55;
    
    /* Use inline asm to pin values in registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
    
    /* Function call that clobbers caller-saved registers */
    int f = ext_func1(a);
    
    /* More computations with different operations */
    int g = (a * b) + (c << 2);
    int h = (d | e) ^ (f & 0xF);
    int i = (g - h) * 3;
    int j = (h + i) / 2;
    
    asm volatile("" : : "r"(f), "r"(g), "r"(h), "r"(i), "r"(j));
    
    /* Another function call */
    int k = ext_func2(g, h);
    
    /* Complex nested expression chain */
    int l = ((a + b) * (c - d)) | ((e & f) ^ (g | h));
    int m = (i << 3) + (j >> 2) - (k * 5);
    int n = (l & m) | (~l & ~m);
    
    asm volatile("" : : "r"(k), "r"(l), "r"(m), "r"(n));
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
}

/* Test function 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(int seed) {
    volatile double dbarrier = (double)seed;
    
    /* Integer computations */
    int i1 = seed * 2;
    int i2 = seed + 100;
    int i3 = seed & 0x7F;
    
    /* Floating-point computations */
    double d1 = dbarrier * 1.5;
    double d2 = dbarrier / 3.14159;
    double d3 = dbarrier + 2.71828;
    
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3));
    asm volatile("" : : "f"(d1), "f"(d2), "f"(d3));
    
    /* Function call with floating point */
    double d4 = ext_func3(d1);
    
    /* More mixed computations */
    int i4 = i1 * i2 + i3;
    double d5 = d2 * d3 - d4;
    int i5 = (i4 & 0xFF) | (seed << 8);
    double d6 = d5 / d4 + dbarrier;
    
    asm volatile("" : : "r"(i4), "r"(i5), "f"(d4), "f"(d5), "f"(d6));
    
    /* Complex expression mixing types */
    double result = (double)i4 * d5 + (double)i5 * d6 - d4;
    
    return result;
}

/* Test function 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* Complex computation in loop body */
            int a = i * j + 7;
            int b = (i ^ j) & 0xFF;
            int c = (i << 3) | (j >> 2);
            
            /* Use asm to prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            /* Function call inside loop */
            int d = ext_func1(a);
            
            /* More computations */
            int e = b * c - d;
            int f = (a & e) | (c ^ d);
            
            asm volatile("" : : "r"(d), "r"(e), "r"(f));
            
            total += a + b + c + d + e + f;
            
            /* Another inner loop with different step */
            for (int k = j; k < N; k += i + 1) {
                int g = k * i - j;
                int h = (k ^ i) & (j | 0xF);
                total += g * h;
            }
        }
    }
    
    return total;
}

/* Test function 4: 64-bit and vector operations */
long long test_64bit_vector(int seed) {
    /* 64-bit operations */
    long long ll1 = (long long)seed * 1000000007LL;
    long long ll2 = ll1 + 1234567890123LL;
    long long ll3 = ll1 ^ ll2;
    
    asm volatile("" : : "r"(ll1), "r"(ll2), "r"(ll3));
    
    /* Function call with 64-bit */
    long long ll4 = ext_func4(ll1);
    
    /* Vector operations */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = v1 + v2;
    v4si v4 = v1 & v2;
    
    /* Extract elements to force register usage */
    int vsum = v3[0] + v3[1] + v3[2] + v3[3];
    int vand = v4[0] | v4[1] | v4[2] | v4[3];
    
    asm volatile("" : : "r"(ll4), "r"(vsum), "r"(vand));
    
    /* More 64-bit computations */
    long long ll5 = ll2 * ll3 / (ll4 + 1);
    long long ll6 = (ll1 << 5) | (ll2 >> 3);
    
    return ll1 + ll2 + ll3 + ll4 + ll5 + ll6 + vsum + vand;
}

/* Test function 5: Extreme register pressure with many temporaries */
int test_extreme_pressure(int seed) {
    /* Create a long chain of independent computations */
    int v01 = seed * 1 + 1;
    int v02 = seed * 2 + 2;
    int v03 = seed * 3 + 3;
    int v04 = seed * 4 + 4;
    int v05 = seed * 5 + 5;
    int v06 = seed * 6 + 6;
    int v07 = seed * 7 + 7;
    int v08 = seed * 8 + 8;
    int v09 = seed * 9 + 9;
    int v10 = seed * 10 + 10;
    
    asm volatile("" : : "r"(v01), "r"(v02), "r"(v03), "r"(v04), "r"(v05),
                       "r"(v06), "r"(v07), "r"(v08), "r"(v09), "r"(v10));
    
    /* Function call in the middle */
    int v11 = ext_func1(v01);
    
    /* More computations */
    int v12 = v02 * v03 - v04;
    int v13 = v05 | v06 & v07;
    int v14 = (v08 ^ v09) + v10;
    int v15 = v11 * 3 + v12;
    
    asm volatile("" : : "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15));
    
    /* Another function call */
    int v16 = ext_func2(v13, v14);
    
    /* Even more computations */
    int v17 = v15 << 2;
    int v18 = v16 >> 1;
    int v19 = v17 & v18;
    int v20 = v19 | 0xFFFF;
    
    /* Complex expression using many values */
    int result = (v01 + v02 + v03 + v04 + v05 + 
                  v06 + v07 + v08 + v09 + v10 +
                  v11 + v12 + v13 + v14 + v15 +
                  v16 + v17 + v18 + v19 + v20);
    
    return result;
}

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x5A5A5A5A; }
int ext_func2(int x, int y) { return (x * y) & 0x7FFFFFFF; }
double ext_func3(double x) { return x * 0.70710678; }
long long ext_func4(long long x) { return x + 0x123456789ABCDEFLL; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Starting early rematerialization tests...\n");
    
    /* Call all test functions to ensure they're not optimized away */
    int result1 = test_high_int_pressure(seed);
    double result2 = test_mixed_pressure(seed);
    long long result3 = test_nested_loops(N);
    long long result4 = test_64bit_vector(seed);
    int result5 = test_extreme_pressure(seed);
    
    /* Use results in a volatile way to prevent dead code elimination */
    volatile int checksum = 0;
    checksum += result1;
    checksum += (int)result2;
    checksum += (int)result3;
    checksum += (int)result4;
    checksum += result5;
    
    printf("Test results: %d, %.2f, %lld, %lld, %d\n", 
           result1, result2, result3, result4, result5);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
