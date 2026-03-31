/* test_early_remat.c - Program to trigger GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x1234; }
int ext_func2(int x, int y) { return x * y + 0x5678; }
double ext_func3(double x) { return x * 1.2345; }
long long ext_func4(long long x) { return x + 0x9ABCDEF0; }

/* Vector type to engage different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with integer operations */
int test_high_pressure_int(int seed) {
    volatile int result = 0;
    int a = seed + 1;
    int b = seed * 2;
    int c = seed / 3;
    int d = seed - 4;
    int e = seed ^ 0x55;
    int f = seed | 0xAA;
    int g = seed & 0xF0;
    int h = seed << 2;
    int i = seed >> 1;
    int j = ~seed;
    
    /* Force values into registers with inline asm */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    int t2 = d & e | f;
    int t3 = g ^ h + i;
    int t4 = j * a - b;
    int t5 = c + d * e;
    int t6 = f & g | h;
    int t7 = i ^ j + a;
    int t8 = b * c - d;
    int t9 = e + f * g;
    int t10 = h & i | j;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5));
    
    /* Function call to clobber registers */
    int r1 = ext_func1(t1);
    int r2 = ext_func2(t2, t3);
    
    /* More computations after call */
    int t11 = t4 * r1 + t5;
    int t12 = t6 & r2 | t7;
    int t13 = t8 ^ t9 + t10;
    int t14 = r1 * t11 - t12;
    int t15 = t13 + r2 * t14;
    
    asm volatile("" : : "r"(t11), "r"(t12), "r"(t13), "r"(t14), "r"(t15));
    
    /* Another function call */
    int r3 = ext_func1(t15);
    
    /* Final computation chain */
    result = t11 + t12 - t13 * t14 / (t15 + 1) ^ r3;
    
    /* Use result to prevent elimination */
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 2: Mixed integer and floating-point operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    
    /* Integer computations */
    int i1 = seed * 3;
    int i2 = seed + 7;
    int i3 = seed ^ 0xFF;
    int i4 = seed | 0xCC;
    
    /* Floating-point computations */
    double f1 = seed * 1.5;
    double f2 = seed / 2.0;
    double f3 = seed + 3.14159;
    double f4 = seed - 2.71828;
    
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(i4));
    asm volatile("" : : "f"(f1), "f"(f2), "f"(f3), "f"(f4));
    
    /* Mixed operations */
    double m1 = i1 * f1;
    double m2 = i2 + f2;
    int m3 = (int)(f3 * i3);
    double m4 = f4 / (i4 + 1);
    
    /* Function call with floating point */
    double r1 = ext_func3(m1);
    
    /* More mixed computations */
    double m5 = m2 * r1 + m4;
    int m6 = m3 ^ (int)m5;
    double m7 = m5 - m4 * 2.0;
    
    asm volatile("" : : "r"(m3), "r"(m6), "f"(m1), "f"(m2), "f"(m4), "f"(m5), "f"(m7));
    
    /* Another call */
    int r2 = ext_func2(m6, (int)m7);
    
    result = m5 + m7 * r1 - r2;
    asm volatile("" : "+f"(result));
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop computations with high pressure */
            int a = i * j + 1;
            int b = (i ^ j) * 3;
            int c = (i & j) | 0xAA;
            int d = (i << 2) + (j >> 1);
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            /* Function call inside inner loop */
            int r1 = ext_func1(a);
            int r2 = ext_func2(b, c);
            
            /* More computations */
            int e = r1 * d - r2;
            int f = (a & b) | (c ^ d);
            int g = r1 + r2 * e;
            
            asm volatile("" : : "r"(e), "r"(f), "r"(g));
            
            /* Another call */
            int r3 = ext_func1(g);
            
            total += e + f - g * r3;
            
            /* Force register pressure with additional temps */
            int t1 = e * 2;
            int t2 = f / 3;
            int t3 = g ^ 0x55;
            int t4 = r3 & 0xF0;
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
        }
        
        /* Outer loop computation */
        long long outer = ext_func4(i);
        total += outer;
    }
    
    asm volatile("" : "+r"(total));
    return total;
}

/* Test 4: 64-bit and vector operations */
long long test_64bit_vector(int seed) {
    volatile long long result = 0;
    
    /* 64-bit operations */
    long long ll1 = (long long)seed * 1000000000LL;
    long long ll2 = (long long)seed << 20;
    long long ll3 = ll1 ^ ll2;
    long long ll4 = ll1 | (0xFEDCBA9876543210LL);
    
    asm volatile("" : : "r"(ll1), "r"(ll2), "r"(ll3), "r"(ll4));
    
    /* Vector operations */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = v1 + v2;
    v4si v4 = v1 & v2;
    
    /* Extract vector elements to force scalar registers */
    int ve1 = v3[0];
    int ve2 = v3[1];
    int ve3 = v3[2];
    int ve4 = v3[3];
    
    asm volatile("" : : "r"(ve1), "r"(ve2), "r"(ve3), "r"(ve4));
    
    /* Mixed 64-bit and 32-bit computations */
    long long m1 = ll1 * ve1;
    long long m2 = ll2 + ve2;
    int m3 = (int)(ll3 >> 32) ^ ve3;
    long long m4 = ll4 / (ve4 + 1);
    
    /* Function call with 64-bit */
    long long r1 = ext_func4(m1);
    
    /* More computations */
    long long m5 = m2 * r1 + m4;
    int m6 = m3 ^ (int)m5;
    long long m7 = m5 - m4 * 2;
    
    asm volatile("" : : "r"(m3), "r"(m6), "r"(m5), "r"(m7));
    
    /* Another call */
    int r2 = ext_func2(m6, (int)m7);
    
    result = m5 + m7 * r1 - r2;
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(int seed) {
    volatile int result = 0;
    
    /* Create many independent values */
    int v[30];
    for (int i = 0; i < 30; i++) {
        v[i] = seed * i + (i ^ 0x55);
    }
    
    /* Force all into registers with inline asm */
    asm volatile("" : : 
        "r"(v[0]), "r"(v[1]), "r"(v[2]), "r"(v[3]), "r"(v[4]),
        "r"(v[5]), "r"(v[6]), "r"(v[7]), "r"(v[8]), "r"(v[9]),
        "r"(v[10]), "r"(v[11]), "r"(v[12]), "r"(v[13]), "r"(v[14]),
        "r"(v[15]), "r"(v[16]), "r"(v[17]), "r"(v[18]), "r"(v[19]),
        "r"(v[20]), "r"(v[21]), "r"(v[22]), "r"(v[23]), "r"(v[24]),
        "r"(v[25]), "r"(v[26]), "r"(v[27]), "r"(v[28]), "r"(v[29]));
    
    /* Long computation chain using all values */
    int t1 = v[0] * v[1] + v[2];
    int t2 = v[3] & v[4] | v[5];
    int t3 = v[6] ^ v[7] + v[8];
    int t4 = v[9] * v[10] - v[11];
    int t5 = v[12] + v[13] * v[14];
    int t6 = v[15] & v[16] | v[17];
    int t7 = v[18] ^ v[19] + v[20];
    int t8 = v[21] * v[22] - v[23];
    int t9 = v[24] + v[25] * v[26];
    int t10 = v[27] & v[28] | v[29];
    
    /* Multiple function calls */
    int r1 = ext_func1(t1);
    int r2 = ext_func2(t2, t3);
    int r3 = ext_func1(t4);
    int r4 = ext_func2(t5, t6);
    
    /* More computations with results */
    int f1 = t7 * r1 + t8;
    int f2 = t9 & r2 | t10;
    int f3 = r3 ^ r4 + t1;
    int f4 = t2 * t3 - t4;
    
    /* Final combination */
    result = f1 + f2 - f3 * f4 / (r1 + r2 + r3 + r4 + 1);
    
    asm volatile("" : "+r"(result));
    return result;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Running early rematerialization tests...\n");
    
    /* Run all tests to maximize coverage */
    int r1 = test_high_pressure_int(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(seed % 50 + 10);
    printf("Test 3 result: %lld\n", r3);
    
    long long r4 = test_64bit_vector(seed);
    printf("Test 4 result: %lld\n", r4);
    
    int r5 = test_extreme_pressure(seed);
    printf("Test 5 result: %d\n", r5);
    
    /* Final checksum to use all results */
    volatile int final = r1 + (int)r2 + (int)r3 + (int)r4 + r5;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
