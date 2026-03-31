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
volatile void* vptr = NULL;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((uintptr_t)h % 256) + (int)i + j;
    
    /* Complex arithmetic mixing types */
    float fsum = b * (float)c + (float)a + (float)*d;
    double dsum = c * (double)b + (double)g + (double)i;
    
    /* Pointer arithmetic */
    if (d != NULL) {
        *d = sum + (int)fsum + (int)dsum;
    }
    
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data24 s2, 
                           void* h, size_t i, unsigned j) {
    /* Use all parameters including structs */
    double result = (double)a + (double)s1.a + (double)s1.b + 
                   (double)s1.c + (double)s1.d + (double)b + c;
    
    if (d != NULL) {
        result += *d;
    }
    
    result += (double)e + (double)f + s2.x + (double)s2.y + 
             (double)s2.z + (double)((uintptr_t)h % 256) + 
             (double)i + (double)j;
    
    /* Modify through pointer */
    if (d != NULL) {
        *d = (int)result;
    }
    
    return result;
}

/* Another 10-parameter function with different signature */
static inline void* func10_alt(void* a, int b, float c, double d, 
                              short e, long f, size_t g, char h, 
                              unsigned i, int* out) {
    /* Complex operations using all parameters */
    int temp = b + (int)c + (int)d + e + (int)f + (int)g + h + i;
    
    if (out != NULL) {
        *out = temp;
    }
    
    /* Return modified pointer */
    return (void*)((uintptr_t)a + temp);
}

/* Another 11-parameter function */
static inline long func11_alt(struct Data24 s1, int a, float b, double c,
                             void* d, short e, struct Data16 s2, long f,
                             size_t g, char h, unsigned i) {
    long result = (long)s1.x + s1.y + s1.z + a + (long)b + (long)c + 
                 (long)((uintptr_t)d % 256) + e + s2.a + s2.b + 
                 f + (long)g + h + i;
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, 
                         long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, 
                           int*, char, short, struct Data24, 
                           void*, size_t, unsigned);
typedef void* (*Func10AltPtr)(void*, int, float, double, short, 
                             long, size_t, char, unsigned, int*);
typedef long (*Func11AltPtr)(struct Data24, int, float, double,
                            void*, short, struct Data16, long,
                            size_t, char, unsigned);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

int main() {
    int result1 = 0, result2 = 0;
    double dresult1 = 0.0, dresult2 = 0.0;
    void* presult = NULL;
    long lresult = 0;
    
    /* Initialize structs */
    struct Data16 s1 = {10, 20, 30.5f, 40.5f};
    struct Data24 s2 = {50.5, 60, 70, {'a', 'b', 'c', 'd'}};
    struct Data16 s3 = {100, 200, 300.5f, 400.5f};
    struct Data24 s4 = {500.5, 600, 700, {'e', 'f', 'g', 'h'}};
    
    int data1 = 42, data2 = 84;
    int* ptr1 = &data1;
    int* ptr2 = &data2;
    
    /* Loop with varying arguments - some from volatile vars */
    for (int i = 0; i < 100; i++) {
        /* Call 10-parameter functions directly */
        result1 += func10(
            v1 + i,            /* int */
            v2 + i,            /* float */
            v3 + i,            /* double */
            ptr1,              /* int* */
            'A' + (i % 26),    /* char */
            i * 2,             /* short */
            i * 1000L,         /* long */
            vptr,              /* void* */
            i * sizeof(int),   /* size_t */
            i * 3              /* unsigned */
        );
        
        /* Call 11-parameter function with structs by value */
        dresult1 += func11(
            v1 + i * 2,
            s1,                /* struct Data16 by value */
            v2 + i * 0.5f,
            v3 + i * 0.25,
            ptr2,
            'Z' - (i % 26),
            i * 3,
            s2,                /* struct Data24 by value */
            (void*)(uintptr_t)i,
            i * sizeof(double),
            i * 4
        );
        
        /* Modify structs slightly each iteration */
        s1.a += 1;
        s2.x += 0.1;
        
        /* Call through function pointers (prevents inlining decisions) */
        if (i % 2 == 0) {
            result2 += func10_array[0](
                i, v2, v3, &data1, 'M', i, i*1000L, 
                (void*)(uintptr_t)v1, i*8, i*5
            );
            
            dresult2 += func11_array[0](
                i, s3, v2*2, v3*2, &data2, 'N', i*2,
                s4, (void*)(uintptr_t)v1, i*16, i*6
            );
        } else {
            /* Use alternate signatures */
            presult = func10_alt(
                (void*)(uintptr_t)i, i*2, v2, v3, i, 
                i*1000L, i*32, 'P', i*7, &data1
            );
            
            lresult += func11_alt(
                s4, i*3, v2*3, v3*3, presult, i*4,
                s3, i*2000L, i*64, 'Q', i*8
            );
        }
        
        /* Prevent loop unrolling from simplifying parameter passing */
        if (i % 10 == 0) {
            ptr1 = (ptr1 == &data1) ? &data2 : &data1;
            ptr2 = (ptr2 == &data1) ? &data2 : &data1;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %.2f, %.2f, %p, %ld\n", 
           result1, result2, dresult1, dresult2, presult, lresult);
    
    return 0;
}
