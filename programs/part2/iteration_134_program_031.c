/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to force register pressure */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Prevent inlining */
__attribute__((noinline)) int helper1(int a, int b) {
    return a * b + (a ^ b);
}

__attribute__((noinline)) double helper2(double a, double b) {
    return a * b - a / b;
}

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
__attribute__((noinline)) int test_integer_pressure(int seed) {
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 4;
    int e = seed * 13 + 5;
    int f = seed * 17 - 6;
    int g = seed * 19 + 7;
    int h = seed * 23 - 8;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = (e & f) | (g ^ h);
    int t3 = t1 * t2 - (a << 2);
    int t4 = (b >> 3) + (c & 0xFF);
    int t5 = t3 ^ t4;
    int t6 = d * e + f * g;
    int t7 = h * a - b * c;
    int t8 = t5 | t6 & t7;
    int t9 = t8 + (d << 1);
    int t10 = t9 * 314159265;
    
    /* Function call to clobber caller-saved registers */
    int r1 = ext_func1(t10);
    
    /* More computations after call */
    int t11 = r1 * a + b * 2;
    int t12 = c * d - e / 3;
    int t13 = (f & g) | (h ^ t11);
    int t14 = t12 + t13 * 7;
    int t15 = t14 - (r1 >> 4);
    
    asm volatile("" : : "r"(t15));
    
    return t15;
}

/* Test 2: Mixed integer and floating-point pressure */
__attribute__((noinline)) double test_mixed_pressure(int seed) {
    double a = seed * 1.1;
    double b = seed * 2.2;
    double c = seed * 3.3;
    double d = seed * 4.4;
    int i1 = seed * 5;
    int i2 = seed * 7;
    int i3 = seed * 11;
    
    /* Force both int and float values */
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3));
    asm volatile("" : : "f"(a), "f"(b), "f"(c), "f"(d));
    
    /* Mixed computations */
    double f1 = a * b + c / d;
    int t1 = i1 * i2 + i3;
    double f2 = f1 * t1 - a;
    int t2 = (i1 & i2) | i3;
    double f3 = f2 + b * t2;
    
    /* External call */
    double r1 = ext_func2(f3);
    
    /* More mixed ops */
    int t3 = t1 * t2 - (int)r1;
    double f4 = r1 * a + b * c;
    int t4 = t3 ^ (i1 << 2);
    double f5 = f4 / d + t4;
    
    asm volatile("" : : "f"(f5), "r"(t4));
    
    return f5;
}

/* Test 3: Nested loops with complex induction */
__attribute__((noinline)) long long test_nested_loops(int N) {
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with register pressure */
            int a = i * j + 1;
            int b = j * 3 - 2;
            int c = a ^ b;
            int d = (a & b) | c;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            /* Function call inside loop */
            int r = ext_func1(d);
            
            /* More computations */
            int e = r * a - b;
            int f = c + d * 2;
            int g = (e & f) ^ r;
            
            sum += g;
            
            /* Additional pressure point */
            if (j % 5 == 0) {
                int h = ext_func1(g);
                sum += h * 2;
            }
        }
        
        /* Outer loop computation */
        long long outer = ext_func3(i);
        sum += outer * i;
    }
    
    return sum;
}

/* Test 4: 64-bit and vector operations */
__attribute__((noinline)) v4si test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    long long ll1 = seed * 1000000000LL;
    long long ll2 = seed * 2000000000LL;
    
    /* Force 64-bit values */
    asm volatile("" : : "r"(ll1), "r"(ll2));
    
    /* Vector operations */
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    v4si v5 = v3 & v4;
    
    /* 64-bit computations */
    long long ll3 = ll1 * ll2 + seed;
    long long ll4 = ll2 - ll1 * 3;
    long long ll5 = (ll3 ^ ll4) + ll1;
    
    /* External call with 64-bit */
    long long r1 = ext_func3(ll5);
    
    /* More vector ops */
    v4si v6 = v5 | v3;
    v4si v7 = v6 * (v4si){r1 & 0xFF, (r1 >> 8) & 0xFF, 
                          (r1 >> 16) & 0xFF, (r1 >> 24) & 0xFF};
    
    asm volatile("" : : "r"(r1));
    
    return v7;
}

/* Test 5: Extreme register pressure with many live values */
__attribute__((noinline)) int test_extreme_pressure(int seed) {
    /* Declare many variables */
    int v1 = seed * 1, v2 = seed * 2, v3 = seed * 3, v4 = seed * 4;
    int v5 = seed * 5, v6 = seed * 6, v7 = seed * 7, v8 = seed * 8;
    int v9 = seed * 9, v10 = seed * 10, v11 = seed * 11, v12 = seed * 12;
    int v13 = seed * 13, v14 = seed * 14, v15 = seed * 15, v16 = seed * 16;
    
    /* Force all into registers */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                       "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                       "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                       "r"(v13), "r"(v14), "r"(v15), "r"(v16));
    
    /* Chain of dependent computations */
    int r1 = v1 * v2 + v3;
    int r2 = v4 & v5 | v6;
    int r3 = r1 ^ r2 + v7;
    int r4 = v8 * v9 - v10;
    int r5 = r3 & r4 | v11;
    int r6 = v12 + v13 * v14;
    int r7 = r5 ^ r6 - v15;
    int r8 = v16 * r7 + r1;
    
    /* Multiple function calls */
    int c1 = ext_func1(r8);
    int c2 = ext_func1(c1 + r2);
    int c3 = ext_func1(c2 * r3);
    
    /* Final computation using many values */
    int result = c1 + c2 * 2 + c3 * 3 + r4 / 4 + r5 * 5 + 
                 r6 - r7 + r8 * 8 + v1 - v2 + v3 * v4;
    
    asm volatile("" : : "r"(result));
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(seed % 50 + 10);
    printf("Test 3 result: %lld\n", r3);
    
    v4si r4 = test_vector_ops(seed);
    printf("Test 4 result: {%d, %d, %d, %d}\n", 
           r4[0], r4[1], r4[2], r4[3]);
    
    int r5 = test_extreme_pressure(seed);
    printf("Test 5 result: %d\n", r5);
    
    /* Prevent dead code elimination */
    volatile int checksum = r1 + (int)r2 + (int)r3 + 
                           r4[0] + r4[1] + r4[2] + r4[3] + r5;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
double ext_func2(double x) { return x * 1.61803398875; }
long long ext_func3(long long x) { return x * 6364136223846793005LL; }
