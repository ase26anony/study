/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Prevent inlining */
__attribute__((noinline)) int dummy_external(int x) {
    return x ^ 0x55AA55AA;
}

/* Vector type to engage different machine modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with integer operations */
__attribute__((noinline)) 
int test_integer_pressure(int seed) {
    volatile int barrier = seed; /* Prevent optimization */
    int a = barrier + 1;
    int b = barrier * 2;
    int c = barrier / 3;
    int d = barrier - 4;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Force into register */
    
    int t2 = b & c | d;
    asm volatile("" : : "r"(t2));
    
    int t3 = (a << 3) ^ (b >> 2);
    asm volatile("" : : "r"(t3));
    
    int t4 = t1 * t2 - t3;
    asm volatile("" : : "r"(t4));
    
    /* Function call creates register pressure */
    int t5 = ext_func1(t4);
    
    int t6 = t2 + t5 * 7;
    asm volatile("" : : "r"(t6));
    
    int t7 = t3 ^ t6 & 0xFF;
    asm volatile("" : : "r"(t7));
    
    int t8 = t4 | (t7 << 16);
    asm volatile("" : : "r"(t8));
    
    /* Another function call */
    int t9 = ext_func2(t8, t6);
    
    int t10 = t5 + t9 * 11;
    asm volatile("" : : "r"(t10));
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t10 = t10 * 3 + i;
        asm volatile("" : : "r"(t10));
    }
    
    return t10;
}

/* Test 2: Mixed integer and floating-point operations */
__attribute__((noinline))
double test_mixed_types(int seed) {
    volatile double dbarrier = seed * 1.5;
    double da = dbarrier + 1.1;
    double db = dbarrier * 2.2;
    float fc = dbarrier / 3.3f;
    float fd = dbarrier - 4.4f;
    
    /* Mix operations to engage different register classes */
    double dt1 = da * db + fc;
    asm volatile("" : : "r"(*(long long*)&dt1)); /* Force FP reg use */
    
    float ft2 = fc * fd - (float)da;
    asm volatile("" : : "r"(*(int*)&ft2));
    
    /* Function call with FP argument */
    double dt3 = ext_func3(dt1);
    
    double dt4 = dt3 * 5.5 + ft2;
    asm volatile("" : : "r"(*(long long*)&dt4));
    
    /* Integer computation in between */
    int it5 = (int)dt4 * 7;
    asm volatile("" : : "r"(it5));
    
    float ft6 = ft2 * it5 / 2.0f;
    asm volatile("" : : "r"(*(int*)&ft6));
    
    return dt4 + ft6;
}

/* Test 3: Nested loops with complex induction variables */
__attribute__((noinline))
long long test_nested_loops(int N) {
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* Complex expression using both i and j */
            int val = (i * 3 + j * 5) ^ (i - j);
            asm volatile("" : : "r"(val));
            
            /* Additional computation to increase pressure */
            int tmp1 = val * 7 + i;
            asm volatile("" : : "r"(tmp1));
            
            int tmp2 = (j << 4) | (i & 0xF);
            asm volatile("" : : "r"(tmp2));
            
            /* Function call inside loop */
            int tmp3 = ext_func1(tmp1 + tmp2);
            asm volatile("" : : "r"(tmp3));
            
            sum += tmp3;
            
            /* More intermediate values */
            for (int k = 0; k < 2; k++) {
                int inner = (tmp3 * k + val) & 0xFF;
                asm volatile("" : : "r"(inner));
                sum += inner;
            }
        }
    }
    
    return sum;
}

/* Test 4: 64-bit and vector operations */
__attribute__((noinline))
v4si test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    
    /* Vector operations */
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    
    /* Scalar operations mixed in */
    long long ll1 = seed * 1000000000LL;
    asm volatile("" : : "r"(ll1));
    
    long long ll2 = ext_func4(ll1);
    asm volatile("" : : "r"(ll2));
    
    v4si v5 = v3 - v4;
    v4si v6 = v5 | v1;
    
    /* More 64-bit operations */
    for (int i = 0; i < 4; i++) {
        ll2 = ll2 * 3 + i;
        asm volatile("" : : "r"(ll2));
    }
    
    return v6 + (v4si){ll2 & 0xFF, (ll2 >> 8) & 0xFF, 
                       (ll2 >> 16) & 0xFF, (ll2 >> 24) & 0xFF};
}

/* Test 5: Extreme register pressure with many live values */
__attribute__((noinline))
int test_extreme_pressure(int seed) {
    /* Create many independent live values */
    int v[20];
    for (int i = 0; i < 20; i++) {
        v[i] = seed * i + i * i;
        asm volatile("" : : "r"(v[i])); /* Force each into register */
    }
    
    /* Chain computations keeping many values live */
    int r1 = v[0] * v[1] + v[2];
    int r2 = v[3] & v[4] | v[5];
    int r3 = v[6] ^ v[7] << v[8];
    int r4 = v[9] + v[10] - v[11];
    
    /* Keep all intermediate results live */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    
    /* Function call with multiple arguments */
    int r5 = ext_func2(r1, r2);
    int r6 = ext_func2(r3, r4);
    
    /* More computations */
    int r7 = r5 * 3 + r6 * 7;
    int r8 = (r1 & r7) | (r2 ^ r6);
    
    asm volatile("" : : "r"(r7), "r"(r8));
    
    /* Use all v[] values in final computation */
    int result = r7;
    for (int i = 0; i < 20; i++) {
        result ^= v[i];
    }
    
    return result + r8;
}

/* External function implementations */
int ext_func1(int x) { return dummy_external(x); }
int ext_func2(int x, int y) { return dummy_external(x + y); }
double ext_func3(double x) { return x * 1.2345; }
long long ext_func4(long long x) { return x ^ 0x123456789ABCDEF0LL; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(seed % 10 + 5);
    printf("Test 3 result: %lld\n", r3);
    
    v4si r4 = test_vector_ops(seed);
    printf("Test 4 result: %d %d %d %d\n", r4[0], r4[1], r4[2], r4[3]);
    
    int r5 = test_extreme_pressure(seed);
    printf("Test 5 result: %d\n", r5);
    
    /* Final checksum to prevent optimization */
    volatile int final = r1 + (int)r2 + (int)r3 + r4[0] + r5;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
