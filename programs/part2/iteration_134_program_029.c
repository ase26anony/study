/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to create pressure points */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Volatile assembly to pin values in registers */
#define PIN_REGISTER(var) asm volatile("" : : "r"(var))
#define PIN_MEMORY(var) asm volatile("" : : "m"(var))

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test_high_pressure_int(int seed) {
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed + 12345;
    int d = seed ^ 0xABCDEF;
    int e = seed | 0x123456;
    int f = seed & 0xF0F0F0F;
    
    PIN_REGISTER(a);
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    int t2 = d ^ e | f;
    int t3 = (a << 3) | (b >> 2);
    int t4 = c * d - e;
    int t5 = f & a ^ b;
    int t6 = t1 + t2 * 3;
    int t7 = t3 / (t4 + 1);
    int t8 = t5 | t6 & t7;
    int t9 = external_func1(t8);  /* Function call creates pressure */
    
    PIN_REGISTER(t1);
    PIN_REGISTER(t2);
    PIN_REGISTER(t3);
    
    int t10 = t9 * 7 + t1;
    int t11 = t2 ^ t3 << 1;
    int t12 = external_func2(t10, t11);
    int t13 = t12 - t4 + t5;
    int t14 = t6 * t7 / 2;
    int t15 = t8 | t13 & t14;
    
    PIN_REGISTER(t4);
    PIN_REGISTER(t5);
    PIN_REGISTER(t6);
    
    /* More computations with function calls */
    int t16 = external_func1(t15);
    int t17 = t16 + t9 * 3;
    int t18 = t10 ^ t11 | t12;
    int t19 = t13 * t14 - t15;
    int t20 = external_func2(t17, t18);
    
    /* Final computation */
    int result = t19 + t20 + t16 + t17 + t18;
    PIN_MEMORY(result);
    
    return result;
}

/* Test 2: Mixed floating-point and integer operations */
double test_mixed_fp_int(int seed) {
    double a = seed * 1.5;
    double b = seed / 3.14159;
    float c = seed * 0.25f;
    float d = seed + 0.5f;
    
    PIN_REGISTER(a);
    PIN_REGISTER(b);
    
    /* Mixed computations */
    double t1 = a * b + c;
    float t2 = c * d - a;
    int t3 = (int)(a + b) * seed;
    double t4 = external_func3(t1);
    
    PIN_REGISTER(c);
    PIN_REGISTER(d);
    
    double t5 = t4 * 2.0 + t2;
    int t6 = t3 ^ seed | (int)t4;
    float t7 = external_func3(t5) * 0.5f;
    double t8 = t5 + t6 * 0.01;
    
    /* Function call between dependent computations */
    int t9 = external_func1(t6);
    double t10 = t8 * t7 - t4;
    float t11 = t2 + t7 / 2.0f;
    
    PIN_REGISTER(t1);
    PIN_REGISTER(t2);
    
    /* More pressure */
    double t12 = external_func3(t10);
    int t13 = external_func2(t9, (int)t11);
    double t14 = t12 * t10 + t11;
    
    double result = t14 + t8 + t4 + t12;
    PIN_MEMORY(result);
    
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        int base = i * 2 + 1;
        PIN_REGISTER(base);
        
        /* Inner loop with non-trivial induction */
        for (int j = i * 2; j < N; j += 3) {
            int k = j * 3 - i;
            long long val1 = base * k + j;
            long long val2 = external_func4(val1);
            
            PIN_REGISTER(k);
            PIN_REGISTER(val1);
            
            /* Complex expression in loop body */
            long long t1 = val1 ^ val2;
            long long t2 = t1 << (j % 16);
            long long t3 = external_func4(t2);
            long long t4 = t3 * base - k;
            
            /* Function call increases pressure */
            int t5 = external_func1((int)t4);
            long long t6 = t4 + t5 * 3LL;
            
            sum += t6;
            
            PIN_REGISTER(t2);
            PIN_REGISTER(t3);
        }
        
        /* Additional computation between loops */
        int mid = external_func1(base);
        long long extra = external_func4(mid);
        sum += extra * i;
    }
    
    PIN_MEMORY(sum);
    return sum;
}

/* Test 4: Vector operations with different modes */
int test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed | 0xFF, seed & 0xF0, seed ^ 0xAA, ~seed};
    
    PIN_REGISTER(v1);
    PIN_REGISTER(v2);
    
    /* Vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v2;
    v4si r3 = r1 | r2;
    v4si r4 = r2 & v3;
    
    /* Scalar extractions increase pressure */
    int s1 = r1[0] + r1[1];
    int s2 = r2[2] * r2[3];
    int s3 = r3[0] ^ r3[1];
    int s4 = r4[2] | r4[3];
    
    PIN_REGISTER(r1);
    PIN_REGISTER(r2);
    
    /* Mix with function calls */
    int t1 = external_func1(s1);
    int t2 = external_func2(s2, s3);
    v4si r5 = {t1, t2, s3, s4};
    
    v4si r6 = r5 * v1 + v2;
    int t3 = external_func1(r6[0]);
    int t4 = external_func2(r6[1], r6[2]);
    
    PIN_REGISTER(r3);
    PIN_REGISTER(r4);
    
    /* Final computation */
    int result = t1 + t2 + t3 + t4 + s1 + s2 + s3 + s4;
    for (int i = 0; i < 4; i++) {
        result += r1[i] + r2[i] + r3[i] + r4[i];
    }
    
    PIN_MEMORY(result);
    return result;
}

/* Test 5: Extreme register pressure with all types */
long long test_extreme_pressure(int seed) {
    /* Use all register classes */
    int i1 = seed;
    long long ll1 = seed * 100LL;
    double d1 = seed * 1.234;
    float f1 = seed * 0.567f;
    v4si vec1 = {seed, seed+1, seed+2, seed+3};
    
    PIN_REGISTER(i1);
    PIN_REGISTER(ll1);
    PIN_REGISTER(d1);
    PIN_REGISTER(f1);
    PIN_REGISTER(vec1);
    
    /* Chain of dependent computations with function calls */
    int i2 = external_func1(i1);
    long long ll2 = ll1 * i2;
    double d2 = external_func3(d1);
    float f2 = f1 * 2.0f;
    
    PIN_REGISTER(i2);
    PIN_REGISTER(ll2);
    
    int i3 = i2 ^ (int)ll2;
    long long ll3 = external_func4(ll2);
    double d3 = d2 * d1 + f2;
    
    /* More mixing */
    v4si vec2 = vec1 + (v4si){i3, (int)ll3, (int)d3, (int)f2};
    int i4 = external_func2(i3, vec2[0]);
    long long ll4 = ll3 + i4;
    
    PIN_REGISTER(d2);
    PIN_REGISTER(f2);
    
    /* Final pressure burst */
    double d4 = external_func3(d3);
    float f3 = external_func3(f2);
    int i5 = external_func1(i4);
    long long ll5 = external_func4(ll4);
    v4si vec3 = vec2 * (v4si){i5, (int)ll5, (int)d4, (int)f3};
    
    /* Combine results */
    long long result = ll5 + i5 + (long long)d4 + (long long)f3;
    for (int i = 0; i < 4; i++) {
        result += vec3[i];
    }
    
    PIN_MEMORY(result);
    return result;
}

/* Dummy external functions */
int external_func1(int x) { return x * 2 + 1; }
int external_func2(int x, int y) { return x ^ y; }
double external_func3(double x) { return x * 1.5; }
long long external_func4(long long x) { return x * 3LL; }

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 1234;
    long long total = 0;
    
    /* Run all tests to ensure code generation */
    total += test_high_pressure_int(seed);
    total += (long long)test_mixed_fp_int(seed);
    total += test_nested_loops(seed % 10 + 5);
    total += test_vector_ops(seed);
    total += test_extreme_pressure(seed);
    
    /* Prevent optimization of total */
    volatile long long final_result = total;
    printf("Result: %lld\n", final_result);
    
    return 0;
}
