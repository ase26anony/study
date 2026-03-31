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
volatile char v4 = 'A';

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((size_t)h % 256) + (int)i + (int)j;
    
    /* Complex arithmetic mixing types */
    float fsum = b * (float)c + (float)a;
    double dsum = c * (double)b + (double)*d;
    
    /* Pointer arithmetic */
    if (d) *d += sum;
    
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, long g, void* h, 
                           size_t i, unsigned j) {
    /* Use all parameters including struct members */
    double sum = (double)a + s1.a + s1.b + s1.c + s1.d + b + c;
    sum += *d + e + f + g + (double)((size_t)h % 256) + i + j;
    
    /* Complex operations */
    sum *= (f % 2 == 0) ? 1.1 : 0.9;
    
    /* Modify through pointer */
    if (d) *d += (int)sum;
    
    return sum;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, struct Data24 s, 
                             char* d, short e, long f, void* g, 
                             size_t h, unsigned i) {
    long result = (long)a + b + (long)c + (long)s.x + s.y + s.z;
    result += (long)(d ? *d : 0) + e + f + (long)((size_t)g % 256) + h + i;
    
    /* Use struct members */
    for (int idx = 0; idx < 4; idx++) {
        result += s.w[idx];
    }
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(struct Data16 s1, struct Data24 s2, int a,
                              float b, double c, int* d, char e, 
                              short f, long g, void* h, size_t i) {
    float result = s1.c + s1.d + (float)s2.x + b;
    result += (float)a + (float)c + (float)*d + (float)e + (float)f + (float)g;
    result += (float)((size_t)h % 256) + (float)i;
    
    /* Complex floating point operations */
    for (int idx = 0; idx < 4; idx++) {
        result += s2.w[idx] * 0.1f;
    }
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func10AltPtr)(double, int, float, struct Data24, char*, short, long, void*, size_t, unsigned);
typedef float (*Func11AltPtr)(struct Data16, struct Data24, int, float, double, int*, char, short, long, void*, size_t);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, NULL};
static Func11Ptr func11_array[] = {func11, NULL};
static Func10AltPtr func10_alt_array[] = {func10_alt, NULL};
static Func11AltPtr func11_alt_array[] = {func11_alt, NULL};

int main() {
    int counter = 0;
    int data1 = 100;
    int data2 = 200;
    char str[] = "test";
    
    /* Initialize structs */
    struct Data16 s16 = {10, 20, 30.5f, 40.5f};
    struct Data24 s24 = {50.5, 60, 70, {'a', 'b', 'c', 'd'}};
    
    /* Loop with varying arguments to trigger different expansion paths */
    for (int i = 0; i < 100; i++) {
        /* Mix of volatile and constant arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + i * 0.5f;
        double arg3 = v3 + i * 0.25;
        char arg5 = v4 + (i % 26);
        short arg6 = (short)(i * 2);
        long arg7 = (long)i * 1000L;
        
        /* Call 10-parameter function directly */
        int res1 = func10(arg1, arg2, arg3, &data1, arg5, arg6, arg7,
                         (void*)(size_t)i, (size_t)(i * 10), (unsigned)(i * 5));
        
        /* Modify struct between calls */
        s16.a += i;
        s16.c += i * 0.1f;
        
        /* Call 11-parameter function directly */
        double res2 = func11(arg1, s16, arg2, arg3, &data2, arg5, arg6, arg7,
                            (void*)(size_t)(i + 1), (size_t)(i * 20), 
                            (unsigned)(i * 10));
        
        /* Modify other struct */
        s24.x += i * 0.5;
        s24.y += i;
        
        /* Call alternative 10-parameter function */
        long res3 = func10_alt(arg3, arg1, arg2, s24, str, arg6, arg7,
                              (void*)(size_t)(i + 2), (size_t)(i * 30),
                              (unsigned)(i * 15));
        
        /* Call alternative 11-parameter function */
        float res4 = func11_alt(s16, s24, arg1, arg2, arg3, &data1, arg5, arg6,
                               arg7, (void*)(size_t)(i + 3), (size_t)(i * 40));
        
        /* Use function pointers to prevent optimization */
        if (i % 3 == 0 && func10_array[0]) {
            res1 = func10_array[0](arg1, arg2, arg3, &data1, arg5, arg6, arg7,
                                  (void*)(size_t)i, (size_t)(i * 10), 
                                  (unsigned)(i * 5));
        }
        
        if (i % 4 == 0 && func11_array[0]) {
            res2 = func11_array[0](arg1, s16, arg2, arg3, &data2, arg5, arg6, arg7,
                                  (void*)(size_t)(i + 1), (size_t)(i * 20),
                                  (unsigned)(i * 10));
        }
        
        /* Accumulate results to prevent elimination */
        counter += res1 + (int)res2 + (int)res3 + (int)res4;
        
        /* Conditional calls to create different control flow */
        if (i % 10 == 0) {
            /* Nested call with different arguments */
            int nested = func10(i, i * 0.5f, i * 0.25, &counter, 'X', 
                               (short)i, (long)i * 100, 
                               (void*)(size_t)i, (size_t)i, (unsigned)i);
            counter += nested;
        }
    }
    
    printf("Result: %d\n", counter);
    return 0;
}
