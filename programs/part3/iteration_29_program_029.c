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
                         short f, long g, void* h, size_t i, unsigned j)
{
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + (*d ? *d : 0) + e + f + (int)g;
    sum += (i % 100) + j;
    
    /* Pointer arithmetic to use pointer parameter */
    if (h) {
        char* ch = (char*)h;
        sum += ch[0];
    }
    
    /* Floating point operations */
    float fsum = b * 2.0f + (float)c;
    return sum + (int)fsum;
}

/* Function with exactly 11 parameters - including struct by value */
static inline double func11(int a, struct Data16 d1, float b, double c, 
                           int* d, char e, short f, long g, void* h, 
                           size_t i, unsigned j)
{
    /* Use all parameters including struct members */
    double total = (double)a + d1.a + d1.b + d1.c + d1.d;
    total += b + c + (*d ? *d : 0.0) + e + f + g;
    
    /* Use pointer parameter */
    if (h) {
        uintptr_t addr = (uintptr_t)h;
        total += (double)(addr % 1000);
    }
    
    total += (double)i + (double)j;
    return total;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, struct Data24 d, 
                             short e, long* f, size_t g, char h, 
                             unsigned i, void* j)
{
    long result = (long)a + b + (long)c + d.y + e + (*f) + (long)g + h + i;
    
    /* Use struct members */
    result += (long)d.x + d.z;
    
    /* Use pointer */
    if (j) {
        result += ((uintptr_t)j) & 0xFF;
    }
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(int a, int b, int c, int d, int e, int f,
                              int g, int h, int i, int j, int k)
{
    /* Force use of all parameters in non-trivial way */
    float avg = (a + b + c + d + e + f + g + h + i + j + k) / 11.0f;
    
    /* Mix with floating operations */
    float variance = 0.0f;
    variance += (a - avg) * (a - avg);
    variance += (b - avg) * (b - avg);
    variance += (c - avg) * (c - avg);
    
    return variance / 3.0f;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func10AltPtr)(double, int, float, struct Data24, short, long*, size_t, char, unsigned, void*);
typedef float (*Func11AltPtr)(int, int, int, int, int, int, int, int, int, int, int);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {func10, NULL};
static Func11Ptr func11_array[] = {func11, NULL};
static Func10AltPtr func10_alt_array[] = {func10_alt, NULL};
static Func11AltPtr func11_alt_array[] = {func11_alt, NULL};

int main(void)
{
    int result_int;
    double result_double;
    long result_long;
    float result_float;
    
    struct Data16 d16 = {10, 20, 30.5f, 40.5f};
    struct Data24 d24 = {100.5, 200, 300, {'a', 'b', 'c', 'd'}};
    
    int local_int = 42;
    float local_float = 3.14f;
    double local_double = 2.71828;
    int* local_ptr = &local_int;
    long local_long = 123456789L;
    void* local_void_ptr = (void*)&local_int;
    
    /* Call functions directly with mixed arguments */
    for (int i = 0; i < 10; i++) {
        /* Use volatile variables to prevent constant propagation */
        int arg1 = v1 + i;
        float arg2 = v2 + i;
        double arg3 = v3 + i;
        int* arg4 = (i % 2) ? local_ptr : v4;
        
        /* Call 10-parameter function */
        result_int = func10(arg1, arg2, arg3, arg4,
                           'A' + i,              /* char */
                           (short)(100 + i),     /* short */
                           local_long + i,       /* long */
                           local_void_ptr,       /* void* */
                           (size_t)(1000 + i),   /* size_t */
                           (unsigned)(500 + i)); /* unsigned */
        
        /* Call 11-parameter function with struct by value */
        struct Data16 temp_d16 = {arg1, arg1 * 2, arg2, arg2 * 2};
        result_double = func11(arg1, temp_d16, arg2, arg3, arg4,
                              'B' + i,
                              (short)(200 + i),
                              local_long + i * 2,
                              local_void_ptr,
                              (size_t)(2000 + i),
                              (unsigned)(1000 + i));
        
        /* Call alternate 10-parameter function */
        result_long = func10_alt(local_double + i,
                                arg1,
                                local_float + i,
                                d24,
                                (short)(300 + i),
                                &local_long,
                                (size_t)(3000 + i),
                                'C' + i,
                                (unsigned)(1500 + i),
                                local_void_ptr);
        
        /* Call alternate 11-parameter function */
        result_float = func11_alt(i, i+1, i+2, i+3, i+4, i+5,
                                 i+6, i+7, i+8, i+9, i+10);
        
        /* Use results to prevent elimination */
        if (result_int > 1000 || result_double > 1000.0 || 
            result_long > 1000L || result_float > 1000.0f) {
            printf("Iteration %d: %d, %.2f, %ld, %.2f\n", 
                   i, result_int, result_double, result_long, result_float);
        }
    }
    
    /* Call through function pointers with volatile guard */
    volatile int selector = v1;
    
    if (selector > 0) {
        /* Call through function pointer arrays */
        if (func10_array[0]) {
            result_int = func10_array[0](100, 200.5f, 300.75, &local_int,
                                        'X', 99, 999L, NULL, 10000, 5000);
        }
        
        if (func11_array[0]) {
            struct Data16 d16_2 = {1, 2, 3.0f, 4.0f};
            result_double = func11_array[0](50, d16_2, 60.5f, 70.75, 
                                           &local_int, 'Y', 88, 888L, 
                                           local_void_ptr, 9999, 8888);
        }
        
        if (func10_alt_array[0]) {
            result_long = func10_alt_array[0](123.456, 789, 321.0f, d24,
                                             66, &local_long, 5555, 'Z',
                                             4444, local_void_ptr);
        }
        
        if (func11_alt_array[0]) {
            result_float = func11_alt_array[0](1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        }
        
        printf("Pointer calls: %d, %.2f, %ld, %.2f\n",
               result_int, result_double, result_long, result_float);
    }
    
    /* Additional calls with constant arguments to allow specialization */
    result_int = func10(1, 2.0f, 3.0, &local_int, 'a', 10, 100L, NULL, 1000, 500);
    result_double = func11(2, d16, 4.0f, 6.0, &local_int, 'b', 20, 200L, 
                          local_void_ptr, 2000, 1000);
    
    return (result_int + (int)result_double) % 256;
}
