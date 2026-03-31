/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Dummy external functions (simulated with simple implementations) */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
double ext_func2(double x) { return x * 1.234567; }
long long ext_func3(long long x) { return x + 0x123456789ABCDEFLL; }

/* Vector type to engage different machine modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed + 0x1234;
    int d = seed ^ 0xABCD;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Pin in register */
    
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 * t2 - a + b;
    asm volatile("" : : "r"(t3));
    
    /* Function call to clobber caller-saved registers */
    int t4 = ext_func1(t3);
    
    int t5 = t4 * 7 + t2 / 3;
    int t6 = (t5 << 3) | (t4 >> 2);
    asm volatile("" : : "r"(t6));
    
    int t7 = t6 ^ t3 + t1;
    int t8 = ext_func1(t7);
    
    int t9 = t8 * 11 - t6 * 13;
    int t10 = (t9 & 0xFF) | (t8 & 0xFF00);
    asm volatile("" : : "r"(t10));
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t10 = t10 * (t9 + i) - t8;
        t9 = t9 ^ (t10 >> (i & 3));
        asm volatile("" : : "r"(t9), "r"(t10));
    }
    
    result = t10;
    return result;
}

/* Test 2: Mixed floating-point and integer pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double f1 = seed * 1.5;
    double f2 = seed / 2.0;
    int i1 = (int)seed * 3;
    int i2 = (int)(seed * 1000) & 0x7FF;
    
    /* Mix float and int operations */
    double ft1 = f1 * f2 + (double)i1;
    asm volatile("" : : "r"(i1), "r"(i2));
    
    int it1 = i1 * i2 + (int)f1;
    double ft2 = ext_func2(ft1);
    
    double ft3 = ft2 * 3.14159 - ft1;
    int it2 = ext_func1(it1);
    
    /* Nested loops with derived induction variables */
    for (int i = 0; i < 4; i++) {
        for (int j = i * 2; j < 8; j += 3) { /* Non-trivial induction */
            ft3 = ft3 * (1.0 + (double)j / 100.0) + ft2;
            it2 = it2 ^ (j * i1 + i2);
            asm volatile("" : : "r"(it2));
        }
        /* Function call inside loop */
        ft2 = ext_func2(ft3);
    }
    
    result = ft3 + (double)it2;
    return result;
}

/* Test 3: 64-bit and vector operations */
long long test_wide_pressure(long long seed) {
    volatile long long result = 0;
    long long ll1 = seed * 3LL;
    long long ll2 = seed + 0x123456789LL;
    long long ll3 = seed ^ 0xFEDCBA9876543210LL;
    
    /* 64-bit operations */
    long long lt1 = ll1 * ll2 - ll3;
    asm volatile("" : : "r"(lt1));
    
    long long lt2 = (ll1 & ll2) | (ll3 << 8);
    long long lt3 = ext_func3(lt1);
    
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2 * (int)(seed & 0xFF);
    asm volatile("" : : "r"(v3));
    
    /* Complex expression chain */
    for (int i = 0; i < 6; i++) {
        lt3 = lt3 * (lt2 + i) - lt1;
        lt2 = lt2 ^ (lt3 >> ((i * 3) & 63));
        lt1 = lt1 + ext_func3(lt2);
        asm volatile("" : : "r"(lt1), "r"(lt2), "r"(lt3));
    }
    
    result = lt3;
    return result;
}

/* Test 4: Extreme pressure with all types */
double test_extreme_pressure(int seed1, double seed2, long long seed3) {
    volatile double final = 0.0;
    
    /* Initialize many variables of different types */
    int ia = seed1, ib = seed1 * 2, ic = seed1 + 100;
    double da = seed2, db = seed2 * 0.5, dc = seed2 / 3.0;
    long long la = seed3, lb = seed3 << 2, lc = seed3 ^ 0xAAAABBBBCCCCDDDDLL;
    
    /* Massive computation sequence */
    for (int outer = 0; outer < 3; outer++) {
        int it = ia * ib - ic + outer;
        double dt = da * db + dc * (double)outer;
        long long lt = la + lb * (outer + 1) - lc;
        
        asm volatile("" : : "r"(it), "r"(lt));
        
        for (int inner = outer * 2; inner < outer * 2 + 4; inner += 1) {
            /* Complex derived induction */
            int derived = inner * 3 + outer * 7;
            it = it ^ (derived * ia);
            dt = dt * ext_func2((double)derived) + db;
            lt = lt & (lc >> (inner & 7)) | (la << (derived & 7));
            
            /* Frequent function calls */
            if (inner % 2 == 0) {
                it = ext_func1(it);
                dt = ext_func2(dt);
            } else {
                lt = ext_func3(lt);
            }
            
            asm volatile("" : : "r"(it), "r"(lt));
        }
        
        ia = it + 1;
        da = dt * 1.01;
        la = lt ^ 0x5555555555555555LL;
    }
    
    final = (double)ia + da + (double)la;
    return final;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    double dseed = (double)seed / 7.0;
    long long llseed = (long long)seed * 1000000009LL;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to create various pressure scenarios */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure(dseed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_wide_pressure(llseed);
    printf("Test 3 result: %lld\n", r3);
    
    double r4 = test_extreme_pressure(seed, dseed * 2.0, llseed / 3);
    printf("Test 4 result: %f\n", r4);
    
    /* Use results to prevent optimization */
    volatile int checksum = r1 + (int)r2 + (int)r3 + (int)r4;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
