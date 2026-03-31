#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

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
    char w[3];
};

/* Mixed parameter types to prevent optimization */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex operations mixing types */
    *d = (int)(b * c) + a;
    return result % 256;
}

static inline double func11(double a, int b, float c, struct Data16 d, 
                           short e, long* f, char g, size_t h, 
                           unsigned i, void* j, int k) {
    /* Use all parameters including struct */
    volatile double result = a + b + c + d.a + d.b + d.c + d.d + e;
    *f = (long)(result * h);
    
    /* Mix struct fields with other params */
    result += (double)((int)j % 256) + g + i + k;
    return result;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(struct Data24 s1, int a, float b, 
                             double c, int* d, char e, short f, 
                             long g, void* h, size_t i) {
    /* Use struct and other params */
    volatile long result = s1.x + s1.y + s1.z + a + (long)b + (long)c;
    result += *d + e + f + g + ((size_t)h % 256) + i;
    
    /* Modify through pointer */
    *d = (int)(s1.x * b);
    return result;
}

/* 11-parameter function with struct in middle */
static inline float func11_mid(int a, float b, double c, int* d, 
                              struct Data16 s, char e, short f, 
                              long g, void* h, size_t i, unsigned j) {
    /* Complex computation using all params */
    volatile float result = b + (float)c + s.c + s.d;
    result += (float)(a + *d + e + f + g + ((int)h % 256) + i + j);
    
    /* Update struct through copy (by value) */
    s.a = (int)(result * 100);
    *d = s.a + s.b;
    
    return result;
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(double, int, float, struct Data16, short, long*, char, size_t, unsigned, void*, int);
typedef long (*Func10AltPtr)(struct Data24, int, float, double, int*, char, short, long, void*, size_t);
typedef float (*Func11MidPtr)(int, float, double, int*, struct Data16, char, short, long, void*, size_t, unsigned);

/* Array of function pointers to prevent direct optimization */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_mid};

/* Volatile variables to prevent constant propagation */
static volatile int v1 = 1;
static volatile float v2 = 2.5f;
static volatile double v3 = 3.14159;
static volatile char v4 = 'A';
static volatile short v5 = 100;
static volatile long v6 = 1000L;
static volatile size_t v7 = 1024;
static volatile unsigned v8 = 255;

int main(int argc, char** argv) {
    int data1 = 42;
    int data2 = 99;
    long data3 = 123456L;
    struct Data16 s1 = {10, 20, 3.14f, 2.718f};
    struct Data24 s2 = {1.234, 567, 89, {'a', 'b', 'c'}};
    
    /* Call functions directly with mixed arguments */
    int result1 = func10(v1, v2, v3, &data1, v4, v5, v6, 
                        (void*)(size_t)v7, v7, v8);
    
    double result2 = func11(v3, v1, v2, s1, v5, &data3, v4, 
                           v7, v8, (void*)(size_t)data1, data2);
    
    long result3 = func10_alt(s2, v1, v2, v3, &data2, v4, v5, 
                             v6, (void*)(size_t)v7, v7);
    
    float result4 = func11_mid(v1, v2, v3, &data1, s1, v4, v5, 
                              v6, (void*)(size_t)v7, v7, v8);
    
    /* Loop with varying calls to trigger different expansions */
    for (int i = 0; i < 10; i++) {
        /* Vary arguments to prevent pattern recognition */
        int idx = i % 2;
        
        /* Call through function pointers - harder to optimize */
        if (i & 1) {
            result1 = func10_array[idx](v1 + i, v2 + i, v3 + i, 
                                       &data1, v4 + i, v5 + i, 
                                       v6 + i, (void*)(size_t)(v7 + i), 
                                       v7 + i, v8 + i);
        } else {
            /* Modify struct between calls */
            s1.a += i;
            s1.c += i;
            
            result2 = func11_array[idx](v3 + i, v1 + i, v2 + i, s1, 
                                       v5 + i, &data3, v4 + i, 
                                       v7 + i, v8 + i, 
                                       (void*)(size_t)(data1 + i), 
                                       data2 + i);
        }
        
        /* Alternate between direct and indirect calls */
        if (i % 3 == 0) {
            result3 = func10_alt(s2, v1 - i, v2 - i, v3 - i, 
                                &data2, v4 - i, v5 - i, 
                                v6 - i, (void*)(size_t)(v7 - i), 
                                v7 - i);
        }
        
        if (i % 4 == 0) {
            s1.b += i;
            result4 = func11_mid(v1 * i, v2 * i, v3 * i, &data1, 
                                s1, v4 + i, v5 + i, v6 + i, 
                                (void*)(size_t)(v7 * i), v7 + i, 
                                v8 * i);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %f, %ld, %f\n", result1, result2, result3, result4);
    printf("Data: %d, %d, %ld\n", data1, data2, data3);
    
    return 0;
}
