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
static volatile int v1 = 1;
static volatile float v2 = 2.0f;
static volatile double v3 = 3.0;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((size_t)h % 256);  /* Use pointer as integer */
    sum += (int)i + (int)j;
    
    /* Complex arithmetic mixing types */
    double result = (double)a * b + c / (*d + 1);
    result += (double)e * f + g;
    
    return sum + (int)result;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(struct Data16 s1, int a, float b, double c, 
                           int* d, char e, short f, long g, void* h, 
                           size_t i, unsigned j) {
    /* Use all parameters including struct members */
    double sum = s1.a + s1.b + s1.c + s1.d;
    sum += a + b + c + *d + e + f + g;
    sum += (double)((size_t)h % 256) + i + j;
    
    /* Complex operations to ensure all parameters are used */
    *d += (int)(sum * 0.5);  /* Modify through pointer */
    
    return sum * (s1.c + 1.0f) / (c + 1.0);
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, short* d, 
                             struct Data24 s, char e, void* f, 
                             size_t g, unsigned h, int i) {
    /* Use struct members */
    long result = (long)a + b + (long)c + *d + e + (long)f + g + h + i;
    result += s.x + s.y + s.z + s.w[0];
    
    /* Modify through pointer */
    *d = (short)(result % 32768);
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(int a, struct Data16 s1, float b, 
                              double c, int* d, struct Data24 s2,
                              char e, short f, long g, void* h, 
                              unsigned i) {
    /* Extensive use of all parameters including both structs */
    float result = a + s1.a + s1.b + s1.c + s1.d;
    result += b + (float)c + *d + e + f + g;
    result += (float)((size_t)h % 256) + i;
    result += (float)(s2.x + s2.y + s2.z + s2.w[0]);
    
    /* Complex floating point operations */
    result = result * s1.c / (b + 1.0f);
    *d = (int)(result * 100.0f);
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(struct Data16, int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func10AltPtr)(double, int, float, short*, struct Data24, char, void*, size_t, unsigned, int);
typedef float (*Func11AltPtr)(int, struct Data16, float, double, int*, struct Data24, char, short, long, void*, unsigned);

/* Array of function pointers */
static Func10Ptr func10_array[2] = {func10, NULL};
static Func11Ptr func11_array[2] = {func11, NULL};
static Func10AltPtr func10_alt_array[2] = {func10_alt, NULL};
static Func11AltPtr func11_alt_array[2] = {func11_alt, NULL};

int main() {
    int data1 = 100;
    int data2 = 200;
    short data3 = 300;
    
    struct Data16 s1 = {10, 20, 30.5f, 40.5f};
    struct Data24 s2 = {50.5, 60, 70, {'a', 'b', 'c', 'd'}};
    
    int total = 0;
    double total_d = 0.0;
    
    /* Call functions directly in a loop - forces multiple expansions */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and constant arguments */
        int arg1 = v1 + i;          /* Volatile-based */
        float arg2 = v2 + i * 0.5f; /* Volatile-based */
        double arg3 = v3 + i * 0.25; /* Volatile-based */
        
        /* Call 10-parameter function */
        total += func10(arg1, arg2, arg3, &data1, 
                       'A' + (i % 26), (short)(i * 2), 
                       (long)i * 3, (void*)(size_t)i, 
                       (size_t)(i * 4), (unsigned)(i * 5));
        
        /* Call 11-parameter function with struct by value */
        total_d += func11(s1, arg1, arg2, arg3, &data2,
                         'B' + (i % 26), (short)(i * 3),
                         (long)i * 4, (void*)(size_t)(i + 1),
                         (size_t)(i * 5), (unsigned)(i * 6));
        
        /* Call alternative functions */
        if (i % 2 == 0) {
            total += func10_alt(arg3, arg1, arg2, &data3, s2,
                               'C' + (i % 26), (void*)(size_t)(i + 2),
                               (size_t)(i * 6), (unsigned)(i * 7), i * 8);
        }
        
        if (i % 3 == 0) {
            total_d += func11_alt(arg1, s1, arg2, arg3, &data1, s2,
                                 'D' + (i % 26), (short)(i * 4),
                                 (long)i * 5, (void*)(size_t)(i + 3),
                                 (unsigned)(i * 8));
        }
    }
    
    /* Call through function pointers - prevents optimization */
    int selector = v1 % 4;
    
    switch (selector) {
        case 0:
            if (func10_array[0]) {
                total += func10_array[0](v1, v2, v3, &data1, 'X', 999, 
                                       1000L, (void*)0x1000, 1024, 2048);
            }
            break;
        case 1:
            if (func11_array[0]) {
                total_d += func11_array[0](s1, v1, v2, v3, &data2, 'Y', 
                                         1000, 2000L, (void*)0x2000, 
                                         4096, 8192);
            }
            break;
        case 2:
            if (func10_alt_array[0]) {
                total += func10_alt_array[0](v3, v1, v2, &data3, s2, 'Z',
                                            (void*)0x3000, 8192, 16384, 32768);
            }
            break;
        case 3:
            if (func11_alt_array[0]) {
                total_d += func11_alt_array[0](v1, s1, v2, v3, &data1, s2,
                                              'W', 2000, 3000L, 
                                              (void*)0x4000, 65535);
            }
            break;
    }
    
    printf("Results: total=%d, total_d=%f\n", total, total_d);
    printf("Modified data: data1=%d, data2=%d, data3=%d\n", data1, data2, data3);
    
    return 0;
}
