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
    int sum = a + (int)b + (int)c;
    if (d) sum += *d;
    sum += e + f + (int)g;
    if (h) sum += (int)((uintptr_t)h & 0xFF);
    sum += (int)i + (int)j;
    
    /* Complex arithmetic mixing types */
    double result = (double)a * b + c / (e + 1);
    if (d) result += *d * 0.5;
    
    return sum + (int)result;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data24 s2, 
                           size_t i, unsigned j, long k) {
    /* Use all parameters extensively */
    double total = (double)a + s1.a + s1.b + s1.c + s1.d;
    total += b + c;
    if (d) total += *d;
    total += e + f + s2.x + s2.y + s2.z;
    total += i + j + k;
    
    /* Complex operations to ensure all params are used */
    for (int idx = 0; idx < 3; idx++) {
        total += s2.w[idx];
    }
    
    /* Mix struct fields in calculations */
    total += (s1.a * s1.c) / (s2.y + 1);
    total += (s2.x * s2.z) / (a + 1);
    
    return total;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, short* d, long e,
                             unsigned f, size_t g, char h, int i, float j) {
    long result = (long)a + b + (long)c + e;
    if (d) result += *d;
    result += f + g + h + i + (long)j;
    
    /* Complex floating point operations */
    double fp_result = a * c * j / (b + 1);
    result += (long)fp_result;
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(struct Data24 s1, int a, float b, double c,
                              int* d, char e, short f, long g, void* h,
                              size_t i, unsigned j) {
    float total = s1.x + s1.y + s1.z;
    for (int idx = 0; idx < 3; idx++) {
        total += s1.w[idx];
    }
    
    total += a + b + (float)c;
    if (d) total += *d;
    total += e + f + g;
    if (h) total += (float)((uintptr_t)h & 0xFF);
    total += i + j;
    
    return total;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, struct Data24, size_t, unsigned, long);
typedef long (*Func10AltPtr)(double, int, float, short*, long, unsigned, size_t, char, int, float);
typedef float (*Func11AltPtr)(struct Data24, int, float, double, int*, char, short, long, void*, size_t, unsigned);

/* Array of function pointers */
Func10Ptr func10_array[2] = {func10, NULL};
Func11Ptr func11_array[2] = {func11, NULL};
Func10AltPtr func10_alt_array[2] = {func10_alt, NULL};
Func11AltPtr func11_alt_array[2] = {func11_alt, NULL};

int main() {
    struct Data16 s1 = {10, 20, 3.14f, 2.718f};
    struct Data24 s2 = {3.14159, 42, 100, {'a', 'b', 'c'}};
    
    int local_int = 100;
    float local_float = 3.14f;
    double local_double = 2.71828;
    int* local_ptr = &local_int;
    short local_short = 32767;
    long local_long = 1000000L;
    size_t local_size = 1024;
    unsigned local_unsigned = 4294967295U;
    
    int total = 0;
    double total_double = 0.0;
    
    /* Loop with varying calls to trigger different expansion paths */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and non-volatile arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + i;
        double arg3 = v3 + i;
        int* arg4 = (i % 2) ? v4 : &local_int;
        
        /* Call 10-parameter function directly */
        if (i % 3 == 0) {
            total += func10(arg1, arg2, arg3, arg4,
                           (char)(i & 0xFF), (short)i, 
                           (long)i * 100, (void*)(uintptr_t)i,
                           (size_t)i, (unsigned)i);
        }
        
        /* Call 11-parameter function directly */
        if (i % 3 == 1) {
            /* Modify structs slightly each iteration */
            s1.a += i;
            s2.y += i;
            
            total_double += func11(arg1, s1, arg2, arg3, arg4,
                                  (char)(i & 0xFF), (short)i, s2,
                                  (size_t)i, (unsigned)i, (long)i * 200);
        }
        
        /* Call through function pointers */
        if (i % 4 == 0 && func10_array[0]) {
            total += func10_array[0](local_int + i, local_float, local_double,
                                    local_ptr, 'A' + (i % 26), local_short,
                                    local_long, (void*)(uintptr_t)local_int,
                                    local_size, local_unsigned);
        }
        
        if (i % 5 == 0 && func11_array[0]) {
            total_double += func11_array[0](local_int, s1, local_float, 
                                           local_double, local_ptr, 'X',
                                           (short)(i % 1000), s2,
                                           local_size, local_unsigned,
                                           local_long);
        }
        
        /* Call alternative functions */
        if (i % 7 == 0) {
            total += func10_alt(local_double, local_int, local_float,
                               (short*)local_ptr, local_long, local_unsigned,
                               local_size, 'Z', i, local_float * 2);
        }
        
        if (i % 11 == 0) {
            total_double += func11_alt(s2, local_int, local_float, local_double,
                                      local_ptr, 'M', (short)(i % 500),
                                      local_long, (void*)(uintptr_t)local_int,
                                      local_size, local_unsigned);
        }
    }
    
    printf("Results: total=%d, total_double=%f\n", total, total_double);
    return 0;
}
