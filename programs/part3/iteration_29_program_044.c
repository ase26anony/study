#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value (forces complex parameter passing) */
struct Data16 {
    int a;
    float b;
    double c;
    char d;
};

struct Data32 {
    long long x;
    long long y;
    float z[2];
    char w;
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
    int sum = a + (int)b + (int)c + *d + e + f + (int)g + (int)(intptr_t)h + (int)i + j;
    
    /* Mix operations with different types */
    float fsum = b * 2.0f + (float)a + (float)c;
    double dsum = c * 3.0 + (double)b + (double)g;
    
    /* Pointer arithmetic */
    if (d) *d += sum;
    
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data32 s2, 
                           size_t i, unsigned j, void* h) {
    /* Use all parameters including struct fields */
    double total = (double)a + s1.b + s1.c + (double)b + c + 
                   (double)*d + (double)e + (double)f + 
                   (double)s2.x + (double)s2.y + s2.z[0] + s2.z[1] +
                   (double)i + (double)j + (double)(intptr_t)h;
    
    /* Modify through pointer */
    if (d) *d += (int)total;
    
    /* Use struct fields in computation */
    total += s1.a * 2.0 + s1.d;
    total += s2.w * 0.5;
    
    return total;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, void* d, 
                             short e, long f, char g, size_t h, 
                             unsigned i, int* j) {
    long result = (long)a + b + (long)c + (long)(intptr_t)d + 
                  e + f + g + (long)h + i + *j;
    
    /* Cross-type operations */
    result += (long)(a * c * (double)b);
    
    if (j) *j ^= (int)result;
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(struct Data32 s1, int a, float b, 
                              double c, int* d, char e, short f, 
                              long g, void* h, size_t i, unsigned j) {
    float total = (float)s1.x + (float)s1.y + s1.z[0] + s1.z[1] + 
                  (float)a + b + (float)c + (float)*d + 
                  (float)e + (float)f + (float)g + 
                  (float)(intptr_t)h + (float)i + (float)j;
    
    total += s1.w * 10.0f;
    
    if (d) *d = (int)total;
    
    return total;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, struct Data32, size_t, unsigned, void*);
typedef long (*Func10AltPtr)(double, int, float, void*, short, long, char, size_t, unsigned, int*);
typedef float (*Func11AltPtr)(struct Data32, int, float, double, int*, char, short, long, void*, size_t, unsigned);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

int main() {
    int counter = 0;
    int data1 = 100;
    int data2 = 200;
    int data3 = 300;
    
    /* Initialize structs */
    struct Data16 s16 = {10, 20.5f, 30.75, 'X'};
    struct Data32 s32 = {1000LL, 2000LL, {1.5f, 2.5f}, 'Y'};
    struct Data32 s32_alt = {3000LL, 4000LL, {3.5f, 4.5f}, 'Z'};
    
    /* Loop to create multiple call sites */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and constant arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + (float)i;
        double arg3 = v3 + (double)i;
        char arg5 = v4 + (char)i;
        
        /* Call 10-parameter functions directly */
        int res1 = func10(arg1, arg2, arg3, &data1, arg5,
                         (short)(i * 2), (long)(i * 3), 
                         (void*)(intptr_t)i, (size_t)(i * 4), 
                         (unsigned)(i * 5));
        
        /* Call 11-parameter function */
        double res2 = func11(arg1, s16, arg2, arg3, &data2, arg5,
                            (short)(i * 2), s32, 
                            (size_t)(i * 4), (unsigned)(i * 5),
                            (void*)(intptr_t)i);
        
        /* Call through function pointer (prevents inlining in some cases) */
        int idx = i % 2;
        int res3 = func10_array[idx](arg1, arg2, arg3, &data3, arg5,
                                    (short)(i * 2), (long)(i * 3),
                                    (void*)(intptr_t)i, (size_t)(i * 4),
                                    (unsigned)(i * 5));
        
        /* Alternate 11-parameter call with different struct */
        double res4 = func11_alt(s32_alt, arg1, arg2, arg3, &data1, arg5,
                                (short)(i * 2), (long)(i * 3),
                                (void*)(intptr_t)i, (size_t)(i * 4),
                                (unsigned)(i * 5));
        
        /* Use results to prevent elimination */
        counter += res1 + (int)res2 + res3 + (int)res4;
        
        /* Modify structs slightly each iteration */
        s16.a++;
        s16.b += 0.1f;
        s32.x += 10;
        s32_alt.y += 20;
    }
    
    /* Conditional calls to create different expansion paths */
    if (counter > 1000) {
        /* Another 10-parameter call with different arguments */
        long res5 = func10_alt(3.14159, 42, 2.71828f, (void*)0x1000,
                              (short)99, 123456L, 'Q', 
                              (size_t)1000, 999, &data2);
        counter += (int)res5;
    }
    
    printf("Result: %d (data1=%d, data2=%d, data3=%d)\n", 
           counter, data1, data2, data3);
    
    return 0;
}
