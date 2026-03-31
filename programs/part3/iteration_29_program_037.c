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
    
    /* Complex arithmetic mixing types */
    double result = (double)a * b + c / (double)(*d + 1);
    result += (double)e * f * g;
    
    /* Return value depends on all parameters */
    return sum + (int)result;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data32 s2, 
                           size_t i, unsigned j, long k) {
    /* Use all parameters including structs */
    double total = (double)a + s1.a + s1.b + s1.c + s1.d;
    total += b + c + (double)(*d) + (double)e + (double)f;
    total += (double)s2.x + (double)s2.y + s2.z + (double)s2.w;
    total += (double)i + (double)j + (double)k;
    
    /* Complex operations with struct fields */
    total += s1.c * s1.d * b;
    total += s2.z / (c + 1.0);
    
    /* Pointer arithmetic */
    total += (double)((intptr_t)d % 1000);
    
    return total;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, void* d, 
                             short e, long f, size_t g, char h, 
                             int* i, unsigned j) {
    /* Mix operations to use all parameters */
    long result = (long)a + b + (long)c + (long)((intptr_t)d);
    result += e * f + g - h;
    result += *i * j;
    
    /* Floating point operations */
    double fp_part = a * (double)c * (double)(*i);
    result += (long)fp_part;
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(struct Data32 s1, int a, float b, 
                              double c, int* d, char e, short f, 
                              long g, void* h, size_t i, unsigned j) {
    float total = (float)s1.x + (float)s1.y + (float)s1.z + (float)s1.w;
    total += (float)a + b + (float)c + (float)(*d) + (float)e;
    total += (float)f + (float)g + (float)((intptr_t)h) + (float)i + (float)j;
    
    /* Complex expression using all parameters */
    total *= 1.0f + b / 100.0f;
    total += (float)((*d) * e * f % 100);
    
    return total;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, struct Data32, size_t, unsigned, long);
typedef long (*Func10AltPtr)(double, int, float, void*, short, long, size_t, char, int*, unsigned);
typedef float (*Func11AltPtr)(struct Data32, int, float, double, int*, char, short, long, void*, size_t, unsigned);

/* Array of function pointers */
Func10Ptr func10_array[2] = {func10, (Func10Ptr)func10_alt};
Func11Ptr func11_array[2] = {func11, (Func11Ptr)func11_alt};

int main() {
    int local_int = 42;
    int* int_ptr = &local_int;
    struct Data16 s16 = {1, 2, 3.0f, 4.0f};
    struct Data32 s32 = {10LL, 20LL, 30.0, 40};
    
    long total = 0;
    double fp_total = 0.0;
    
    /* Call functions multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and non-volatile arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + (float)i;
        double arg3 = v3 + (double)i;
        char arg4 = v4 + (char)(i % 26);
        
        /* Call 10-parameter functions */
        int result10 = func10(
            arg1, arg2, arg3, int_ptr,
            arg4, (short)i, (long)i * 2,
            (void*)(intptr_t)i, (size_t)i * 3, (unsigned)i * 4
        );
        
        total += result10;
        
        /* Call 11-parameter function with structs by value */
        struct Data16 s16_var = {i, i+1, (float)i+2, (float)i+3};
        struct Data32 s32_var = {(long long)i, (long long)i*2, (double)i*3, i*4};
        
        double result11 = func11(
            arg1, s16_var, arg2, arg3,
            int_ptr, arg4, (short)i,
            s32_var, (size_t)i * 5, (unsigned)i * 6, (long)i * 7
        );
        
        fp_total += result11;
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (i % 2 == 0) {
            long alt_result = func10_alt(
                (double)arg1, arg1, arg2, (void*)(intptr_t)arg1,
                (short)arg1, (long)arg1, (size_t)arg1, (char)arg1,
                int_ptr, (unsigned)arg1
            );
            total += alt_result;
        }
        
        /* Modify structs for next iteration */
        s16.a++;
        s16.c += 0.5f;
        s32.x += 5;
        s32.z += 1.5;
    }
    
    /* Call functions through pointer arrays based on runtime condition */
    int selector = total % 4;
    
    switch (selector) {
        case 0:
            fp_total += func10_array[0](
                v1, v2, v3, int_ptr,
                v4, (short)v1, (long)v1 * 2,
                (void*)(intptr_t)v1, (size_t)v1 * 3, (unsigned)v1 * 4
            );
            break;
        case 1:
            fp_total += func11_array[0](
                v1, s16, v2, v3,
                int_ptr, v4, (short)v1,
                s32, (size_t)v1 * 5, (unsigned)v1 * 6, (long)v1 * 7
            );
            break;
        case 2:
            total += func10_array[1](
                v3, v1, v2, (void*)(intptr_t)v1,
                (short)v1, (long)v1, (size_t)v1, (char)v1,
                int_ptr, (unsigned)v1
            );
            break;
        case 3:
            fp_total += func11_array[1](
                s32, v1, v2, v3,
                int_ptr, v4, (short)v1,
                (long)v1, (void*)(intptr_t)v1, (size_t)v1, (unsigned)v1
            );
            break;
    }
    
    printf("Results: total = %ld, fp_total = %f\n", total, fp_total);
    return 0;
}
