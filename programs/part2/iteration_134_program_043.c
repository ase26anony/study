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

/* Test 1: High register pressure with integer operations */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0x12345678;
    int d = seed + 777;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Force t1 to be in register */
    
    int t2 = (c & d) | (a ^ b);
    asm volatile("" : : "r"(t2));
    
    int t3 = external_func1(t1); /* Function call creates pressure */
    
    int t4 = t2 * t3 - a;
    asm volatile("" : : "r"(t4));
    
    int t5 = (b << 3) | (c >> 2);
    int t6 = t4 ^ t5;
    asm volatile("" : : "r"(t6));
    
    int t7 = external_func2(t3, t6);
    
    int t8 = t7 * 7 + t1;
    int t9 = t8 & 0xFFFF;
    asm volatile("" : : "r"(t9));
    
    int t10 = t9 - t4 + t2;
    int t11 = t10 * 3;
    asm volatile("" : : "r"(t11));
    
    int t12 = external_func1(t11);
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t12 = t12 * 2 + i;
        asm volatile("" : : "r"(t12));
        t12 = external_func2(t12, i);
    }
    
    result = t12;
    return result;
}

/* Test 2: Mixed integer and floating point operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.234;
    double d2 = seed / 3.456;
    float f1 = seed * 0.789f;
    float f2 = seed + 123.456f;
    
    int i1 = seed * 5;
    int i2 = seed + 999;
    
    /* Mix operations */
    double t1 = d1 * d2 + f1 * f2;
    asm volatile("" : : "r"(i1), "r"(i2)); /* Keep integers in registers */
    
    int i3 = i1 * i2 + seed;
    asm volatile("" : : "r"(i3));
    
    double t2 = external_func3(t1);
    
    float t3 = f1 * 2.5f - f2;
    asm volatile("" : : "r"(i3)); /* Keep i3 alive */
    
    int i4 = external_func1(i3);
    
    double t4 = t2 + t3 + i4;
    asm volatile("" : : "r"(i4));
    
    /* Nested loops with different induction variables */
    for (int i = 0; i < 100; i++) {
        for (int j = i * 2; j < i * 2 + 50; j += 3) {
            double temp = t4 * j + i;
            i4 = i4 ^ (j * i);
            asm volatile("" : : "r"(i4));
            t4 = external_func3(temp);
        }
        /* Function call in loop increases pressure */
        i4 = external_func2(i4, i);
    }
    
    result = t4 + i4;
    return result;
}

/* Test 3: 64-bit and vector operations */
long long test_64bit_vector(int seed) {
    v4si vec1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si vec2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    long long ll1 = (long long)seed * 1000000000LL;
    long long ll2 = (long long)seed * 2000000000LL;
    
    /* Vector operations */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    /* 64-bit operations */
    long long ll3 = ll1 * ll2 + seed;
    asm volatile("" : : "r"(ll3));
    
    long long ll4 = external_func4(ll3);
    
    /* Mix with function calls */
    for (int i = 0; i < 50; i++) {
        /* Complex induction variable */
        for (long long j = i * 3LL; j < i * 3LL + 20; j += 2) {
            ll4 = ll4 * j + i;
            vec3 = vec3 + vec4;
            asm volatile("" : : "r"(ll4));
            
            /* Function call with 64-bit argument */
            ll4 = external_func4(ll4);
        }
        
        /* More register pressure */
        long long temp = ll4 ^ (ll1 * i);
        int itemp = (int)temp;
        itemp = external_func1(Itemp);
        ll4 = ll4 + itemp;
    }
    
    /* Extract result from vector */
    long long result = ll4;
    for (int i = 0; i < 4; i++) {
        result += vec3[i];
    }
    
    return result;
}

/* Test 4: Extreme register pressure with many live values */
int test_extreme_pressure(int seed) {
    /* Create many independent variables */
    int v[20];
    for (int i = 0; i < 20; i++) {
        v[i] = seed * (i + 1) + i * i;
    }
    
    /* Long computation chain with many live values */
    int r1 = v[0] * v[1] + v[2];
    int r2 = v[3] & v[4] | v[5];
    int r3 = external_func1(r1);
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3));
    
    int r4 = r2 ^ r3 + v[6];
    int r5 = v[7] * 3 - v[8];
    int r6 = external_func2(r4, r5);
    asm volatile("" : : "r"(r4), "r"(r5), "r"(r6));
    
    int r7 = r6 << 2 | r1 >> 1;
    int r8 = v[9] + v[10] * v[11];
    int r9 = external_func1(r7);
    asm volatile("" : : "r"(r7), "r"(r8), "r"(r9));
    
    int r10 = r8 & r9 ^ r4;
    int r11 = v[12] * 7 + v[13];
    int r12 = external_func2(r10, r11);
    asm volatile("" : : "r"(r10), "r"(r11), "r"(r12));
    
    /* Nested loops with complex induction */
    for (int i = 0; i < 10; i++) {
        for (int j = i * 3; j < i * 3 + 15; j += 2) {
            /* Use many live values in computation */
            r12 = r12 * j + r1 + r4 + r7 + r10;
            asm volatile("" : : "r"(r12));
            r12 = external_func1(r12);
        }
        /* Function call that uses multiple values */
        r12 = external_func2(r12, v[i % 20]);
    }
    
    return r12;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to trigger different patterns */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_64bit_vector(seed);
    printf("Test 3 result: %lld\n", r3);
    
    int r4 = test_extreme_pressure(seed);
    printf("Test 4 result: %d\n", r4);
    
    /* Final computation using all results */
    int final = r1 + (int)r2 + (int)r3 + r4;
    printf("Final checksum: %d\n", final);
    
    /* Prevent dead code elimination */
    volatile int dummy = final;
    
    return 0;
}
