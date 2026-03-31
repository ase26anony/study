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
    char w[3];
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
    int sum = a + (int)b + (int)c + (*d ? *d : 0) + e + f + (int)g;
    sum += (h != NULL) ? 1 : 0;
    sum += (int)i + (int)j;
    
    /* Mix operations to create complex RTL patterns */
    double prod = b * c * (d ? *d : 1.0);
    float div = (float)c / (b != 0.0f ? b : 1.0f);
    
    return sum + (int)prod + (int)div;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data24 s2, 
                           size_t i, unsigned j, long k) {
    /* Use all parameters including struct fields */
    double result = a + s1.a + s1.b + s1.c + s1.d + b + c;
    
    if (d) result += *d;
    result += e + f + s2.x + s2.y + s2.z + i + j + k;
    
    /* Complex floating point operations */
    result = result * b / (c != 0.0 ? c : 1.0);
    result += (s1.c * s1.d) - (s2.x * s2.y);
    
    return result;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, void* d, 
                             short e, long f, int* g, char h, 
                             size_t i, struct Data16 s) {
    long base = (long)a + b + (long)c + (long)d + e + f;
    if (g) base += *g;
    base += h + i + s.a + s.b;
    
    /* Mix integer and floating point */
    float temp = (float)a * c * s.c;
    base += (long)temp;
    
    return base;
}

/* Another 11-parameter function */
static inline float func11_alt(int a, int b, int c, float d, double e,
                              short f, long g, void* h, int* i,
                              struct Data24 s, unsigned j) {
    float sum = a + b + c + d + (float)e + f + g;
    sum += (h != NULL) ? 1.0f : 0.0f;
    if (i) sum += *i;
    sum += s.x + s.y + s.z + j;
    
    /* Complex expression */
    sum = sum * d / (e != 0.0 ? (float)e : 1.0f);
    sum += s.w[0] + s.w[1] + s.w[2];
    
    return sum;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, struct Data24, size_t, unsigned, long);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

int main() {
    struct Data16 s1 = {10, 20, 30.5f, 40.5f};
    struct Data24 s2 = {100.5, 200, 300, {'a', 'b', 'c'}};
    
    int local_int = 42;
    int* int_ptr = &local_int;
    float local_float = 3.14f;
    double local_double = 2.71828;
    
    int total = 0;
    double total_d = 0.0;
    
    /* Call functions directly with mixed arguments */
    for (int i = 0; i < 100; i++) {
        /* Use volatile variables to prevent constant propagation */
        int arg1 = v1 + i;
        float arg2 = v2 + i * 0.5f;
        double arg3 = v3 + i * 0.1;
        
        /* Direct calls - may be inlined */
        total += func10(arg1, arg2, arg3, int_ptr, 
                       'A' + (i % 26), (short)i, (long)i * 2,
                       (void*)&s1, (size_t)i, (unsigned)i * 3);
        
        total_d += func11(arg1, s1, arg2, arg3, int_ptr,
                         'B' + (i % 26), (short)(i * 2), s2,
                         (size_t)i * 4, (unsigned)i * 5, (long)i * 6);
        
        /* Alternate functions */
        total += func10_alt(arg3, arg1, arg2, (void*)&s2,
                           (short)(i * 3), (long)i * 4, int_ptr,
                           'C' + (i % 26), (size_t)i * 7, s1);
        
        total_d += func11_alt(arg1, arg1 + 1, arg1 + 2, arg2, arg3,
                             (short)(i * 5), (long)i * 6, (void*)&s1,
                             int_ptr, s2, (unsigned)i * 8);
        
        /* Call through function pointers - prevents inlining in some cases */
        if (i % 3 == 0) {
            total += func10_array[i % 2](arg1, arg2, arg3, int_ptr,
                                        'D' + (i % 26), (short)i, (long)i,
                                        (void*)&s1, (size_t)i, (unsigned)i);
        } else {
            total_d += func11_array[i % 2](arg1, s1, arg2, arg3, int_ptr,
                                          'E' + (i % 26), (short)i, s2,
                                          (size_t)i, (unsigned)i, (long)i);
        }
        
        /* Modify structs to create varying arguments */
        s1.a++;
        s1.c += 0.5f;
        s2.x += 0.1;
        s2.y++;
    }
    
    /* Conditional calls with different parameter counts */
    if (total > 1000) {
        total += func10(1, 2.0f, 3.0, &total, 'X', 10, 100L, 
                       NULL, 1000UL, 2000U);
    } else {
        total_d += func11(2, s1, 4.0f, 5.0, &total, 'Y', 20, s2,
                         2000UL, 4000U, 5000L);
    }
    
    printf("Results: total=%d, total_d=%f\n", total, total_d);
    return 0;
}
