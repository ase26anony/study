#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct Data16 {
    int a;
    int b;
    float c;
    float d;
};

struct Data24 {
    double x;
    int y;
    short z;
    char w[4];
};

/* Volatile variables to prevent constant propagation */
volatile int v1 = 1;
volatile float v2 = 2.0f;
volatile double v3 = 3.0;
volatile int* v4 = NULL;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c;
    if (d) sum += *d;
    sum += e + f + (int)g;
    if (h) sum += (int)((uintptr_t)h & 0xFF);
    sum += (int)i + (int)j;
    
    /* Complex arithmetic mixing types */
    double result = (double)a * b * c;
    if (d) result += *d;
    result += (double)e * f * g;
    
    return (int)result + sum;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data24 s2, 
                           size_t i, unsigned j, long k) {
    /* Use all parameters including structs */
    double sum = (double)a + s1.a + s1.b + s1.c + s1.d;
    sum += b + c;
    if (d) sum += *d;
    sum += e + f;
    sum += s2.x + s2.y + s2.z;
    sum += i + j + k;
    
    /* Complex operations with struct fields */
    double prod = (double)a * s1.c * b * c;
    prod += s2.x * s2.y;
    
    return sum * prod;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(float a, double b, int c, short d, 
                             struct Data16 s, long e, size_t f, 
                             unsigned g, int* h, char i) {
    long result = (long)(a * 100) + (long)b;
    result += c + d + s.a + s.b;
    result += e + (long)f + (long)g;
    if (h) result += *h;
    result += i;
    
    /* Mix in floating point operations */
    double fp_result = (double)a * b * s.c * s.d;
    return result + (long)fp_result;
}

/* Another 11-parameter function */
static inline float func11_alt(int a, int b, int c, double d, float e,
                              struct Data24 s1, short f, long g, 
                              size_t h, unsigned i, struct Data16 s2) {
    float result = (float)a + b + c;
    result += (float)d + e;
    result += (float)s1.x + s1.y + s1.z;
    result += f + g + h + i;
    result += s2.a + s2.b + s2.c + s2.d;
    
    /* Complex floating point chain */
    double chain = (double)a * b * c * d * e;
    chain *= s1.x * s2.c;
    
    return result * (float)chain;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, 
                        long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, 
                           int*, char, short, struct Data24, 
                           size_t, unsigned, long);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

int main() {
    struct Data16 s1 = {10, 20, 30.5f, 40.5f};
    struct Data24 s2 = {50.5, 60, 70, {'a', 'b', 'c', 'd'}};
    struct Data16 s3 = {80, 90, 100.5f, 110.5f};
    struct Data24 s4 = {120.5, 130, 140, {'e', 'f', 'g', 'h'}};
    
    int arr[5] = {1, 2, 3, 4, 5};
    int result = 0;
    double dresult = 0.0;
    
    /* Call functions directly in a loop - forces multiple expansions */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and constant arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + i;
        double arg3 = v3 + i;
        int* arg4 = (i % 2) ? &arr[i % 5] : NULL;
        
        /* Call 10-parameter function */
        result += func10(arg1, arg2, arg3, arg4,
                        (char)(i + 'a'), (short)(i * 2),
                        (long)(i * 100), (void*)(uintptr_t)i,
                        (size_t)(i * 10), (unsigned)(i * 3));
        
        /* Call 11-parameter function */
        dresult += func11(arg1, s1, arg2, arg3, arg4,
                         (char)(i + 'A'), (short)(i * 3), s2,
                         (size_t)(i * 20), (unsigned)(i * 4),
                         (long)(i * 200));
        
        /* Alternate structs on different iterations */
        if (i % 2 == 0) {
            result += func10_alt(arg2, arg3, arg1, (short)i,
                                s3, (long)(i * 50), (size_t)(i * 30),
                                (unsigned)(i * 5), arg4, (char)(i + '0'));
        } else {
            dresult += func11_alt(arg1, arg1 + 1, arg1 + 2, arg3, arg2,
                                 s4, (short)(i * 4), (long)(i * 300),
                                 (size_t)(i * 40), (unsigned)(i * 6), s1);
        }
    }
    
    /* Call through function pointers - prevents optimization */
    int selector = v1 % 2;
    
    for (int i = 0; i < 50; i++) {
        /* Use volatile to prevent compile-time resolution */
        selector = (selector + v1) % 2;
        
        /* Call 10-parameter function through pointer */
        result += func10_array[selector](
            v1 + i, v2 + i, v3 + i, 
            (selector == 0) ? (int*)&v1 : NULL,
            (char)('m' + i), (short)(i * 5),
            (long)(i * 400), (void*)(uintptr_t)(v1 + i),
            (size_t)(i * 60), (unsigned)(i * 7));
        
        /* Call 11-parameter function through pointer */
        dresult += func11_array[selector](
            v1 + i, (selector == 0) ? s1 : s3,
            v2 + i, v3 + i, 
            (selector == 0) ? (int*)&v1 : NULL,
            (char)('M' + i), (short)(i * 6),
            (selector == 0) ? s2 : s4,
            (size_t)(i * 70), (unsigned)(i * 8),
            (long)(i * 500));
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %f\n", result, dresult);
    
    /* Additional complex calling patterns */
    {
        /* Nested calls with mixed parameters */
        int temp = func10(
            func10(1, 2.0f, 3.0, &result, 'x', 10, 100L, 
                  (void*)0x1000, 1000UL, 2000U),
            4.0f, 5.0, NULL, 'y', 20, 200L,
            (void*)0x2000, 2000UL, 4000U);
        
        printf("Nested result: %d\n", temp);
    }
    
    return 0;
}
