/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy external functions */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return x * y + (x ^ y); }
double external_func3(double x) { return x * 1.23456789; }
long long external_func4(long long x) { return x * 3 + 1; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int barrier = seed; /* Prevent optimization */
    int a = barrier + 1;
    int b = barrier * 2;
    int c = barrier / 3;
    int d = barrier - 4;
    int e = barrier ^ 0x12345678;
    int f = barrier | 0x87654321;
    int g = barrier & 0xF0F0F0F0;
    int h = barrier << 3;
    int i = barrier >> 2;
    int j = ~barrier;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = e & f | g ^ h;
    int t3 = i * j + a * c;
    int t4 = b / (d + 1) + e;
    int t5 = f << (g & 7);
    int t6 = h >> (i % 8);
    int t7 = (j + a) * (b - c);
    int t8 = d ^ e ^ f ^ g;
    int t9 = (h | i) & (j | a);
    int t10 = b * c * d * e;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5));
    
    /* Function call that clobbers registers */
    int r1 = external_func1(t1);
    int r2 = external_func2(t2, t3);
    
    /* More computations after call */
    int t11 = t1 * r1 + t2;
    int t12 = t3 ^ r2 ^ t4;
    int t13 = t5 * 7 + t6 / 3;
    int t14 = t7 & t8 | t9;
    int t15 = t10 + r1 - r2;
    
    asm volatile("" : : "r"(t11), "r"(t12), "r"(t13), "r"(t14), "r"(t15));
    
    /* Another function call */
    int r3 = external_func1(t11);
    
    /* Final computations */
    int result = t11 + t12 + t13 + t14 + t15 + r1 + r2 + r3;
    
    /* Use volatile store to prevent elimination */
    volatile int final_result = result;
    return final_result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(int seed) {
    volatile double barrier_d = seed * 1.2345;
    int barrier_i = seed;
    
    /* Integer computations */
    int i1 = barrier_i * 2;
    int i2 = barrier_i + 100;
    int i3 = barrier_i ^ 0xABCD;
    int i4 = barrier_i | 0x1234;
    
    /* Floating computations */
    double d1 = barrier_d * 1.1;
    double d2 = barrier_d / 2.2;
    double d3 = barrier_d + 3.3;
    double d4 = barrier_d - 4.4;
    float f1 = barrier_d * 0.5f;
    float f2 = barrier_d + 1.5f;
    
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(i4));
    asm volatile("" : : "f"(d1), "f"(d2), "f"(d3), "f"(d4));
    asm volatile("" : : "f"(f1), "f"(f2));
    
    /* Mixed computations */
    double t1 = d1 * d2 + i1;
    double t2 = d3 - d4 * i2;
    float t3 = f1 * f2 + i3;
    double t4 = external_func3(d1) * i4;
    
    /* Function call clobbers floating and integer registers */
    int r1 = external_func2(i1, i2);
    double r2 = external_func3(d2);
    
    /* More mixed computations */
    double t5 = t1 * r2 + r1;
    double t6 = t2 / r2 - i3;
    double t7 = t3 * 2.0 + t4;
    
    asm volatile("" : : "f"(t1), "f"(t2), "f"(t3), "f"(t4), 
                       "f"(t5), "f"(t6), "f"(t7));
    
    return t5 + t6 + t7;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop computations with many temporaries */
            int t1 = i * j;
            int t2 = i ^ j;
            int t3 = i | j;
            int t4 = i & j;
            int t5 = i + j * 2;
            int t6 = j - i * 3;
            int t7 = (i << 2) + (j >> 1);
            int t8 = i * i + j * j;
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4),
                               "r"(t5), "r"(t6), "r"(t7), "r"(t8));
            
            /* Function call in inner loop - high pressure */
            if (j % 7 == 0) {
                int r = external_func1(t1);
                t1 += r;
            }
            
            /* More computations */
            int t9 = t1 * t2 + t3;
            int t10 = t4 ^ t5 ^ t6;
            int t11 = t7 & t8 | t9;
            int t12 = t10 * 3 - t11;
            
            sum += t9 + t10 + t11 + t12;
            
            /* Force spill/remat points */
            asm volatile("" : : "r"(t9), "r"(t10), "r"(t11), "r"(t12));
        }
        
        /* Outer loop computation */
        long long outer_tmp = external_func4(i);
        sum += outer_tmp;
    }
    
    return sum;
}

/* Test 4: 64-bit and vector operations for different modes */
v4si test_vector_ops(int seed) {
    /* 64-bit operations */
    long long ll1 = seed * 1000000000LL;
    long long ll2 = seed * 2000000000LL;
    long long ll3 = seed * 3000000000LL;
    long long ll4 = seed * 4000000000LL;
    
    asm volatile("" : : "r"(ll1), "r"(ll2), "r"(ll3), "r"(ll4));
    
    long long ll5 = ll1 * ll2 + ll3;
    long long ll6 = ll2 / (ll4 + 1) + ll1;
    long long ll7 = ll3 ^ ll4 ^ ll1;
    long long ll8 = external_func4(ll1) * ll2;
    
    /* Vector operations */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed | 1, seed | 2, seed | 3, seed | 4};
    v4si v4 = {seed & 0xF, seed & 0xF0, seed & 0xF00, seed & 0xF000};
    
    /* Vector computations */
    v4si v5 = v1 + v2;
    v4si v6 = v3 * v4;
    v4si v7 = v1 & v2;
    v4si v8 = v3 | v4;
    
    /* Mix with scalar */
    int s1 = seed * 5;
    int s2 = seed + 10;
    v4si v9 = v5 * s1;
    v4si v10 = v6 + s2;
    
    /* Function call between vector ops */
    long long r1 = external_func4(ll5);
    v4si v11 = v7 * (int)r1;
    
    /* Final vector result */
    v4si result = v8 + v9 + v10 + v11;
    
    /* Use volatile to prevent elimination */
    volatile v4si final_result = result;
    return final_result;
}

/* Test 5: Extreme register pressure with many live ranges */
int test_extreme_pressure(int seed) {
    /* Create many independent variables */
    int v[50];
    for (int i = 0; i < 50; i++) {
        v[i] = seed + i * 7919; /* Prime multiplier for diversity */
    }
    
    /* Force all into registers with inline asm */
    asm volatile("" : : 
        "r"(v[0]), "r"(v[1]), "r"(v[2]), "r"(v[3]), "r"(v[4]),
        "r"(v[5]), "r"(v[6]), "r"(v[7]), "r"(v[8]), "r"(v[9]),
        "r"(v[10]), "r"(v[11]), "r"(v[12]), "r"(v[13]), "r"(v[14]),
        "r"(v[15]), "r"(v[16]), "r"(v[17]), "r"(v[18]), "r"(v[19]),
        "r"(v[20]), "r"(v[21]), "r"(v[22]), "r"(v[23]), "r"(v[24]));
    
    /* Complex computation graph */
    int r1 = external_func1(v[0]);
    int r2 = external_func2(v[1], v[2]);
    
    int t1 = v[3] * v[4] + r1;
    int t2 = v[5] ^ v[6] ^ r2;
    int t3 = v[7] | v[8] & v[9];
    int t4 = v[10] << (v[11] % 16);
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    /* More function calls */
    int r3 = external_func1(t1);
    int r4 = external_func2(t2, t3);
    
    /* Continue computations */
    int t5 = t1 * r3 + t2;
    int t6 = t3 ^ r4 ^ t4;
    int t7 = v[12] * v[13] - v[14];
    int t8 = v[15] & v[16] | v[17];
    
    /* Final result using many values */
    int result = t5 + t6 + t7 + t8 + r1 + r2 + r3 + r4;
    
    for (int i = 18; i < 30; i++) {
        result += v[i];
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different parts of the pass */
    int r1 = test_high_int_pressure(seed);
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
    
    /* Final checksum */
    int final = r1 + (int)r2 + (int)r3 + r4[0] + r5;
    printf("Final checksum: %d\n", final);
    
    return final != 0 ? 0 : 1;
}
