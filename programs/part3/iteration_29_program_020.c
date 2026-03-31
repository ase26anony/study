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

struct Data32 {
    long long x;
    long long y;
    double z;
    int w;
};

/* Volatile variables to prevent constant propagation */
static volatile int v1 = 1;
static volatile float v2 = 2.0f;
static volatile double v3 = 3.0;
static volatile char v4 = 'A';

/* Function with exactly 10 parameters */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((size_t)h % 256) + (int)i + j;
    
    /* Mix operations with different types */
    float fsum = b * (float)c + (float)a;
    double dsum = c * (double)b + (double)g;
    
    /* Pointer arithmetic */
    if (d) *d += sum;
    
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters */
static inline double func11(double a, int b, float c, long d, short e,
                           void* f, size_t g, unsigned h, char i, 
                           int* j, struct Data16 s) {
    /* Use all parameters */
    double result = a + (double)b + (double)c + (double)d + (double)e;
    result += (double)((size_t)f % 1000) + (double)g + (double)h + (double)i;
    
    /* Use struct members */
    result += s.a + s.b + s.c + s.d;
    
    /* Pointer operation */
    if (j) *j += (int)result;
    
    /* Mixed-type computation */
    result = result * c / (a + 1.0);
    
    return result;
}

/* Another 10-parameter function with struct parameter */
static inline long func10_struct(int a, struct Data16 s1, double b, 
                                float c, int* d, short e, long f,
                                void* g, size_t h, unsigned i) {
    long result = a + s1.a + s1.b + (long)b + (long)c;
    result += e + f + (long)((size_t)g % 256) + (long)h + i;
    
    if (d) *d += (int)result;
    
    /* Use struct float members */
    result += (long)(s1.c * s1.d);
    
    return result;
}

/* 11-parameter function with two structs */
static inline float func11_structs(struct Data16 s1, struct Data32 s2,
                                  int a, float b, double c, int* d,
                                  char e, short f, long g, void* h,
                                  size_t i) {
    float result = s1.c + s1.d + (float)s2.z;
    result += b + (float)c + (float)a + (float)e + (float)f + (float)g;
    
    result += (float)(s2.x % 100) + (float)(s2.y % 100) + (float)s2.w;
    
    if (d) *d += (int)result;
    
    /* Complex mixed computation */
    result = result * b / (s1.c + 1.0f);
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(double, int, float, long, short, void*, size_t, unsigned, char, int*, struct Data16);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_struct};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_structs};

int main() {
    int counter = 0;
    int data1 = 100;
    int data2 = 200;
    struct Data16 s1 = {10, 20, 30.5f, 40.5f};
    struct Data32 s2 = {1000LL, 2000LL, 300.0, 400};
    
    /* Loop to generate multiple call sites */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and constant arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + (float)i;
        double arg3 = v3 + (double)i;
        char arg4 = v4 + (char)(i % 26);
        
        /* Call 10-parameter function directly */
        int res1 = func10(arg1, arg2, arg3, &data1, 
                         arg4, (short)i, (long)i * 2,
                         (void*)(size_t)i, (size_t)i * 3, 
                         (unsigned)i * 4);
        
        /* Call 11-parameter function directly */
        double res2 = func11(arg3, arg1, arg2, (long)i * 5,
                            (short)(i + 1), (void*)(size_t)(i + 2),
                            (size_t)i * 6, (unsigned)i * 7,
                            (char)(arg4 + 1), &data2, s1);
        
        /* Call through function pointers (prevents inlining in some cases) */
        int idx = i % 2;
        int res3 = func10_array[idx](arg1 + 1, arg2 + 1.0f, arg3 + 1.0,
                                    &data1, arg4 + 2, (short)(i + 3),
                                    (long)i * 8, (void*)(size_t)(i + 4),
                                    (size_t)i * 9, (unsigned)i * 10);
        
        double res4 = func11_array[idx](arg3 + 2.0, arg1 + 2, arg2 + 2.0f,
                                       (long)i * 11, (short)(i + 5),
                                       (void*)(size_t)(i + 6),
                                       (size_t)i * 12, (unsigned)i * 13,
                                       (char)(arg4 + 3), &data2, s1);
        
        /* Call struct version directly */
        long res5 = func10_struct(arg1, s1, arg3, arg2, &data1,
                                 (short)i, (long)i * 14,
                                 (void*)(size_t)i, (size_t)i * 15,
                                 (unsigned)i * 16);
        
        float res6 = func11_structs(s1, s2, arg1, arg2, arg3, &data2,
                                   arg4, (short)i, (long)i * 17,
                                   (void*)(size_t)i, (size_t)i * 18);
        
        /* Use results to prevent elimination */
        counter += res1 + (int)res2 + res3 + (int)res4 + (int)res5 + (int)res6;
        
        /* Modify structs for next iteration */
        s1.a++;
        s1.c += 0.5f;
        s2.x += i;
        s2.z += 0.1;
    }
    
    printf("Result: %d\n", counter);
    printf("Data1: %d, Data2: %d\n", data1, data2);
    
    return 0;
}
