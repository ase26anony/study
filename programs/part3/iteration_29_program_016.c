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
    double x;
    double y;
    int z;
    int w;
    char pad[8];
};

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile double g_volatile_double = 2.71828;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex arithmetic mixing types */
    float float_res = b * (float)c + (float)a;
    double double_res = c * (double)b + (double)result;
    
    /* Pointer arithmetic */
    if (d) {
        *d = result + (int)float_res;
    }
    
    return result + (int)float_res + (int)double_res;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data32 s2, 
                           size_t i, unsigned j, long k) {
    /* Use all parameters extensively */
    double result = (double)a + (double)s1.a + (double)s1.b + s1.c + s1.d;
    result += b + c + (double)*d + (double)e + (double)f;
    result += s2.x + s2.y + (double)s2.z + (double)s2.w;
    result += (double)i + (double)j + (double)k;
    
    /* Modify through pointer */
    if (d) {
        *d = (int)result + s1.a + s2.z;
    }
    
    /* Complex floating point operations */
    result = result * b / c + s1.c * s1.d - s2.x * s2.y;
    
    return result;
}

/* Another 10-parameter function with different signature */
static inline void* func10_alt(void* p1, int a, float b, double c, 
                              struct Data16 s, int* d, char e, 
                              short f, long g, size_t h) {
    /* Use parameters */
    int calc = a + (int)b + (int)c + s.a + s.b + (int)s.c + (int)s.d;
    calc += *d + e + f + (int)g + (int)h;
    
    if (d) {
        *d = calc;
    }
    
    /* Return pointer with offset based on calculations */
    return (char*)p1 + (calc % 256);
}

/* Another 11-parameter function */
static inline int func11_alt(double a, double b, double c, int d, int e,
                            float f, float g, struct Data32 s, 
                            short h, long i, void* j) {
    /* Complex floating point computations */
    double sum = a + b + c + s.x + s.y;
    float prod = f * g * (float)s.z;
    
    int result = (int)(sum * prod) + d + e + h + (int)i;
    
    /* Use pointer parameter */
    if (j) {
        *(int*)j = result;
    }
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, struct Data32, size_t, unsigned, long);
typedef void* (*Func10AltPtr)(void*, int, float, double, struct Data16, int*, char, short, long, size_t);
typedef int (*Func11AltPtr)(double, double, double, int, int, float, float, struct Data32, short, long, void*);

/* Array of function pointers - prevents direct call optimization */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

int main() {
    int data1 = 100, data2 = 200, data3 = 300;
    int* ptr1 = &data1;
    int* ptr2 = &data2;
    int* ptr3 = &data3;
    
    struct Data16 s16 = {1, 2, 3.0f, 4.0f};
    struct Data32 s32 = {1.5, 2.5, 3, 4};
    
    /* Volatile to prevent constant folding */
    volatile int selector = 0;
    
    int total = 0;
    double total_d = 0.0;
    
    /* Loop with multiple calls to high-arity functions */
    for (int i = 0; i < 100; i++) {
        /* Vary parameters to prevent optimization */
        int a = i + g_volatile_int;
        float b = (float)i * g_volatile_float;
        double c = (double)i / g_volatile_double;
        
        /* Call 10-parameter function directly */
        int res10 = func10(a, b, c, ptr1, 
                          (char)(i % 128), 
                          (short)(i * 2),
                          (long)i * 3,
                          (void*)(uintptr_t)i,
                          (size_t)(i * 4),
                          (unsigned)(i * 5));
        
        /* Update structs */
        s16.a = i;
        s16.c = b;
        s32.x = c;
        s32.z = i * 2;
        
        /* Call 11-parameter function directly */
        double res11 = func11(a, s16, b, c, ptr2,
                             (char)(i % 64),
                             (short)(i * 3),
                             s32,
                             (size_t)(i * 6),
                             (unsigned)(i * 7),
                             (long)(i * 8));
        
        /* Call through function pointer - obfuscates for compiler */
        if (selector++ % 2 == 0) {
            res10 += func10_array[0](a, b, c, ptr3,
                                    (char)(i % 96),
                                    (short)(i * 4),
                                    (long)i * 5,
                                    (void*)(uintptr_t)(i * 2),
                                    (size_t)(i * 8),
                                    (unsigned)(i * 9));
        } else {
            /* Call alternate 11-parameter function */
            struct Data32 s32_alt = {c, b, i, i*2};
            res11 += func11_alt((double)a, (double)b, c,
                               i, i*2,
                               b, b*2.0f,
                               s32_alt,
                               (short)(i % 256),
                               (long)(i * 10),
                               ptr1);
        }
        
        total += res10;
        total_d += res11;
        
        /* Modify pointers to create aliasing concerns */
        if (i % 10 == 0) {
            ptr1 = &data2;
            ptr2 = &data3;
            ptr3 = &data1;
        } else if (i % 7 == 0) {
            ptr1 = &data1;
            ptr2 = &data2;
            ptr3 = &data3;
        }
    }
    
    /* Call with struct parameters */
    struct Data16 s16_final = {total % 100, total % 200, 
                              (float)total_d, (float)(total_d * 2)};
    struct Data32 s32_final = {total_d, total_d / 2.0, total, total * 2};
    
    /* Final calls with all parameters from volatiles */
    int final10 = func10(g_volatile_int, 
                        g_volatile_float,
                        g_volatile_double,
                        &total,
                        'X',
                        4096,
                        999999L,
                        &total_d,
                        (size_t)total,
                        (unsigned)total_d);
    
    double final11 = func11(g_volatile_int,
                           s16_final,
                           g_volatile_float * 2,
                           g_volatile_double * 3,
                           &total,
                           'Y',
                           8192,
                           s32_final,
                           (size_t)(total * 2),
                           (unsigned)(total_d * 3),
                           123456789L);
    
    printf("Results: total=%d, total_d=%f, final10=%d, final11=%f\n",
           total, total_d, final10, final11);
    
    return (total + (int)total_d + final10 + (int)final11) % 256;
}
