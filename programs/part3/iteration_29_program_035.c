#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value */
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
    char w[5];
};

/* Volatile variables to prevent constant propagation */
volatile int v1 = 1;
volatile float v2 = 2.0f;
volatile double v3 = 3.0;
volatile char v4 = 'A';

/* Function with exactly 10 parameters */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters in non-trivial ways */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((uintptr_t)h % 256);
    sum += (int)i + j;
    
    /* Mix operations to prevent optimization */
    float fsum = b * (float)c + (float)a;
    double dsum = c * (double)b - (double)f;
    
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters */
static inline double func11(int a, float b, double c, struct Data16 d, 
                           int* e, char f, short g, long h, 
                           void* i, size_t j, unsigned k) {
    /* Use all parameters */
    double result = (double)a + (double)b + c;
    result += (double)d.a + (double)d.b + (double)d.c + (double)d.d;
    result += (double)*e + (double)f + (double)g + (double)h;
    result += (double)((uintptr_t)i % 1024);
    result += (double)j + (double)k;
    
    /* Complex mixing of operations */
    result = result * c - (double)b / (a + 1);
    result += (d.c > 0.0f) ? (double)d.d : (double)d.a;
    
    return result;
}

/* Another 11-parameter function with different types */
static inline long func11_alt(double a, int b, float c, struct Data24 d,
                             short* e, char f, int g, float h,
                             void* i, size_t j, unsigned k) {
    long result = (long)a + b + (long)c;
    result += (long)d.x + d.y + d.z + d.w[0];
    result += *e + f + g + (long)h;
    result += (long)((uintptr_t)i % 512);
    result += (long)j + k;
    
    /* Use struct members */
    for (int idx = 0; idx < 5; idx++) {
        result += d.w[idx];
    }
    
    return result * (g + 1);
}

/* 10-parameter function with struct parameter */
static inline float func10_struct(struct Data16 a, int b, float c, double d,
                                 int* e, char f, short g, long h,
                                 void* i, size_t j) {
    float result = (float)a.a + (float)a.b + a.c + a.d;
    result += (float)b + c + (float)d;
    result += (float)*e + (float)f + (float)g + (float)h;
    result += (float)((uintptr_t)i % 256);
    result += (float)j;
    
    /* Conditional operations to prevent optimization */
    if (a.c > 0.0f) {
        result *= 1.5f;
    }
    if (*e > 0) {
        result += 2.0f;
    }
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, float, double, struct Data16, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func11AltPtr)(double, int, float, struct Data24, short*, char, int, float, void*, size_t, unsigned);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, NULL};
static Func11Ptr func11_array[] = {func11, NULL};
static Func11AltPtr func11_alt_array[] = {func11_alt, NULL};

int main() {
    int data1 = 100;
    int data2 = 200;
    short data3 = 300;
    
    struct Data16 d16 = {10, 20, 30.0f, 40.0f};
    struct Data24 d24 = {50.0, 60, 70, {'H', 'e', 'l', 'l', 'o'}};
    
    int total = 0;
    double total_d = 0.0;
    long total_l = 0;
    
    /* Call functions multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        /* Use volatile variables to prevent constant propagation */
        int arg1 = v1 + i;
        float arg2 = v2 + (float)i;
        double arg3 = v3 + (double)i;
        char arg5 = v4 + (char)(i % 26);
        
        /* Call 10-parameter function */
        total += func10(arg1, arg2, arg3, &data1, arg5, 
                       (short)(i * 2), (long)i * 3, 
                       (void*)(uintptr_t)i, (size_t)(i * 4), 
                       (unsigned)(i * 5));
        
        /* Call 10-parameter function with struct */
        d16.a += i;
        d16.c += (float)i;
        total += (int)func10_struct(d16, arg1, arg2, arg3, &data2,
                                   arg5, (short)i, (long)(i * 2),
                                   (void*)(uintptr_t)(i + 1), (size_t)(i * 3));
        
        /* Call 11-parameter function */
        total_d += func11(arg1, arg2, arg3, d16, &data1, arg5,
                         (short)(i * 2), (long)(i * 3),
                         (void*)(uintptr_t)i, (size_t)(i * 4),
                         (unsigned)(i * 5));
        
        /* Call alternate 11-parameter function */
        d24.y += i;
        d24.w[0] += (char)(i % 26);
        total_l += func11_alt(arg3, arg1, arg2, d24, &data3, arg5,
                             i, arg2 + 1.0f,
                             (void*)(uintptr_t)(i * 2), (size_t)(i * 3),
                             (unsigned)(i * 4));
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (func10_array[0] != NULL) {
            total += func10_array[0](arg1 + 1, arg2 + 1.0f, arg3 + 1.0,
                                    &data1, arg5 + 1, (short)(i * 3),
                                    (long)(i * 4), (void*)(uintptr_t)(i * 2),
                                    (size_t)(i * 5), (unsigned)(i * 6));
        }
        
        if (i % 10 == 0) {
            /* Mix in some different patterns */
            total += func10(i, i * 2.0f, i * 3.0, &data2, 'X',
                           (short)i, (long)(i * 10), NULL,
                           (size_t)i, (unsigned)(i * 20));
        }
    }
    
    printf("Results: %d, %f, %ld\n", total, total_d, total_l);
    return 0;
}
