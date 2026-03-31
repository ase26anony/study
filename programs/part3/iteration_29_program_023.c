#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

/* Small structs to pass by value (forcing complex parameter passing) */
struct Data16 {
    int a;
    int b;
    float c;
    float d;
};

struct Data32 {
    long long x;
    long long y;
    double z;
    int w;
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
    sum += (int)((size_t)h % 256);  /* Use pointer as integer */
    sum += (int)i + (int)j;
    
    /* Mix operations to create complex RTL */
    *d = sum * a;
    return sum + (*d) / (a ? a : 1);
}

/* Another 10-parameter function with struct parameter */
static double func10_struct(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, void* h, unsigned j) {
    /* Use struct members */
    double result = s1.a + s1.b + s1.c + s1.d;
    result += a + b + c + *d + e + f;
    result += (double)((size_t)h % 256) + j;
    
    *d = (int)result;
    return result * c;
}

/* Function with exactly 11 parameters */
static inline long func11(int a, float b, double c, int* d, char e,
                         short f, long g, void* h, size_t i, unsigned j,
                         struct Data32 s2) {
    /* Use all 11 parameters */
    long total = a + (long)b + (long)c + *d + e + f + g;
    total += (long)((size_t)h % 256) + (long)i + (long)j;
    total += (long)s2.x + (long)s2.y + (long)s2.z + s2.w;
    
    /* Complex operation mixing parameters */
    *d = (int)(total % 1000);
    return total * g / (a ? a : 1);
}

/* Another 11-parameter variant */
static float func11_mixed(double a, int b, float c, short* d, long e,
                         struct Data16 s1, void* f, size_t g, char h,
                         unsigned i, int j) {
    float result = (float)a + b + c + *d + (float)e;
    result += s1.a + s1.b + s1.c + s1.d;
    result += (float)((size_t)f % 256) + (float)g + h + i + j;
    
    *d = (short)result;
    return result * c;
}

/* Typedefs for function pointers */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Data32);
typedef float (*Func11MixedPtr)(double, int, float, short*, long, struct Data16, void*, size_t, char, unsigned, int);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_struct};
static Func11Ptr func11_array[] = {func11};
static Func11MixedPtr func11_mixed_array[] = {func11_mixed};

int main() {
    int data1 = 100;
    int data2 = 200;
    short data3 = 300;
    
    struct Data16 s1 = {10, 20, 30.5f, 40.5f};
    struct Data32 s2 = {1000LL, 2000LL, 3000.0, 400};
    
    int result10, result11;
    double result10_d;
    float result11_f;
    
    /* Call 10-parameter functions in a loop with varying arguments */
    for (int i = 0; i < 10; i++) {
        /* Use volatile variables to prevent constant propagation */
        int arg1 = v1 + i;
        float arg2 = v2 + i;
        double arg3 = v3 + i;
        
        /* Call through function pointer */
        int idx = i % 2;
        result10 = func10_array[idx](
            arg1, arg2, arg3, &data1,
            (char)('A' + i), (short)(100 + i),
            (long)(1000 + i), (void*)(0x1000 + i),
            (size_t)(10000 + i), (unsigned)(500 + i)
        );
        
        /* Direct call to 10-parameter function with struct */
        result10_d = func10_struct(
            arg1, s1, arg2, arg3, &data2,
            (char)('B' + i), (short)(200 + i),
            (void*)(0x2000 + i), (unsigned)(600 + i)
        );
    }
    
    /* Call 11-parameter functions */
    for (int i = 0; i < 5; i++) {
        int arg1 = v1 * i;
        float arg2 = v2 * i;
        double arg3 = v3 * i;
        
        /* Call through function pointer array */
        result11 = func11_array[0](
            arg1, arg2, arg3, &data1,
            (char)('C' + i), (short)(300 + i),
            (long)(2000 + i), (void*)(0x3000 + i),
            (size_t)(20000 + i), (unsigned)(700 + i),
            s2
        );
        
        /* Call mixed 11-parameter function */
        result11_f = func11_mixed(
            arg3, arg1, arg2, &data3,
            (long)(3000 + i), s1,
            (void*)(0x4000 + i), (size_t)(30000 + i),
            (char)('D' + i), (unsigned)(800 + i),
            i * 100
        );
    }
    
    /* Conditional calls to create different expansion paths */
    if (v1 > 0) {
        result10 = func10(
            v1, v2, v3, &data1,
            v4, 500, 6000L,
            (void*)0x5000, 70000, 800
        );
    } else {
        result11 = func11(
            v1 * 2, v2 * 2, v3 * 2, &data2,
            v4 + 1, 600, 7000L,
            (void*)0x6000, 80000, 900,
            s2
        );
    }
    
    printf("Results: %d, %d, %f, %ld, %f\n", 
           data1, data2, result10_d, result11, result11_f);
    
    return 0;
}
