/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy implementations to satisfy linker */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double external_func3(double x) { return x * 3.141592653589793; }
long long external_func4(long long x) { return x * 0xDEADBEEF; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0x1234;
    int d = seed + 777;
    
    /* Force values into registers with inline asm */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = (a & b) | (c ^ d);
    int t3 = (a << 3) + (b >> 2) * c;
    int t4 = external_func1(t1);  /* Function call creates pressure point */
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    int t5 = t2 * t3 + t4;
    int t6 = t1 ^ t2 ^ t3 ^ t4;
    int t7 = external_func2(t5, t6);
    int t8 = (t5 << 1) | (t6 >> 1);
    
    asm volatile("" : : "r"(t5), "r"(t6), "r"(t7), "r"(t8));
    
    int t9 = t7 * 31 + t8 * 17;
    int t10 = (t7 & 0xFF) | (t8 & 0xFF00);
    int t11 = external_func1(t9);
    int t12 = t10 ^ t11;
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t12 = t12 * 3 + i;
        asm volatile("" : : "r"(t12));
    }
    
    result = t12;
    return result;
}

/* Test 2: Mixed floating-point and integer pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double f1 = seed * 1.5;
    double f2 = seed / 3.14159;
    int i1 = (int)seed * 7;
    int i2 = (int)(seed * 2.0) ^ 0xABCD;
    
    asm volatile("" : : "r"(i1), "r"(i2), "f"(f1), "f"(f2));
    
    double f3 = f1 * f2 + external_func3(f1);
    int i3 = i1 * i2 + external_func1(i1);
    
    asm volatile("" : : "f"(f3), "r"(i3));
    
    /* Complex expression mixing types */
    double f4 = f3 * (double)i3;
    int i4 = (int)f3 * i3;
    
    /* Nested loops with derived induction variables */
    for (int outer = 0; outer < 4; outer++) {
        double acc = 0.0;
        /* Inner loop with non-trivial induction */
        for (int inner = outer * 2; inner < outer * 2 + 8; inner += 3) {
            acc += f4 * inner + i4 / (inner + 1);
            /* Force intermediate to register */
            asm volatile("" : : "f"(acc));
        }
        f4 += acc;
    }
    
    result = f4 + i4;
    return result;
}

/* Test 3: Long long and vector operations */
long long test_wide_types(long long seed) {
    volatile long long result = 0;
    long long ll1 = seed * 0x123456789ABCDEFLL;
    long long ll2 = external_func4(seed);
    long long ll3 = ll1 ^ ll2;
    
    asm volatile("" : : "r"(ll1), "r"(ll2), "r"(ll3));
    
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    
    /* Force vectors to be considered */
    asm volatile("" : : "x"(v3), "x"(v4));
    
    /* Complex chain of long long operations */
    long long ll4 = ll3 * 31;
    long long ll5 = external_func4(ll4);
    long long ll6 = ll4 ^ ll5;
    
    /* Multiple dependent computations */
    for (int i = 0; i < 16; i++) {
        ll6 = ll6 * 3 + i;
        ll6 = ll6 ^ (ll6 >> 32);
        asm volatile("" : : "r"(ll6));
    }
    
    /* Mix with vector results */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += v3[i] + v4[i];
    }
    
    result = ll6 + sum;
    return result;
}

/* Test 4: Extreme register pressure with many live values */
int test_extreme_pressure(int seed) {
    volatile int checksum = 0;
    
    /* Create many independent values */
    int v[32];
    for (int i = 0; i < 32; i++) {
        v[i] = seed * i + i * i;
    }
    
    /* Force all to registers temporarily */
    asm volatile("" : : 
        "r"(v[0]), "r"(v[1]), "r"(v[2]), "r"(v[3]),
        "r"(v[4]), "r"(v[5]), "r"(v[6]), "r"(v[7]),
        "r"(v[8]), "r"(v[9]), "r"(v[10]), "r"(v[11]),
        "r"(v[12]), "r"(v[13]), "r"(v[14]), "r"(v[15]));
    
    /* Complex computation graph */
    int t1 = v[0] * v[1] + v[2] - v[3];
    int t2 = v[4] ^ v[5] | v[6] & v[7];
    int t3 = external_func1(v[8]);
    int t4 = v[9] << v[10] >> v[11];
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    /* More computations with function calls */
    for (int i = 12; i < 20; i += 2) {
        t1 = external_func2(t1, v[i]);
        t2 = t2 * v[i+1] + t1;
        asm volatile("" : : "r"(t1), "r"(t2));
    }
    
    /* Final mixing */
    checksum = t1 ^ t2 ^ t3 ^ t4;
    for (int i = 20; i < 32; i++) {
        checksum = checksum * 31 + v[i];
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int result = 0;
    double dresult = 0.0;
    long long llresult = 0;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different modes */
    result = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", result);
    
    dresult = test_mixed_pressure((double)seed);
    printf("Test 2 result: %f\n", dresult);
    
    llresult = test_wide_types(seed);
    printf("Test 3 result: %lld\n", llresult);
    
    result = test_extreme_pressure(seed);
    printf("Test 4 result: %d\n", result);
    
    /* Use results to prevent optimization */
    volatile int final = result + (int)dresult + (int)llresult;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
