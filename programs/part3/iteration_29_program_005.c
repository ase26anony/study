#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct SmallStruct {
    int x;
    int y;
    float z;
    char w;
};

struct MediumStruct {
    double a;
    int b;
    short c;
    char d;
    float e;
};

/* Mixed parameter types to prevent optimization */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + (int)j;
    return result;
}

static inline double func11(int a, float b, double c, int* d, char e,
                           short f, long g, void* h, size_t i, unsigned j,
                           struct SmallStruct s) {
    /* Complex usage of all parameters */
    double result = (double)a + (double)b + c + (double)*d + (double)e;
    result += (double)f + (double)g + (double)((size_t)h % 256);
    result += (double)i + (double)j + (double)s.x + (double)s.y + (double)s.z;
    return result;
}

/* Another 10-parameter function with different types */
static inline long func10_alt(double a, int b, float c, short* d, long e,
                             unsigned f, size_t g, char h, int i, void* j) {
    long result = (long)a + b + (long)c + *d + e + (long)f + (long)g + h + i;
    result += (long)((size_t)j % 1024);
    return result;
}

/* Another 11-parameter function with struct parameter */
static inline float func11_alt(struct MediumStruct m, int a, float b, double c,
                              int* d, char e, short f, long g, void* h,
                              size_t i, unsigned j) {
    float result = m.a + (float)m.b + m.e + (float)a + b + (float)c;
    result += (float)*d + (float)e + (float)f + (float)g;
    result += (float)((size_t)h % 256) + (float)i + (float)j;
    return result;
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct SmallStruct);
typedef long (*Func10AltPtr)(double, int, float, short*, long, unsigned, size_t, char, int, void*);
typedef float (*Func11AltPtr)(struct MediumStruct, int, float, double, int*, char, short, long, void*, size_t, unsigned);

/* Function pointer array to prevent direct optimization */
static volatile Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static volatile Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

/* Helper to create structs */
static struct SmallStruct make_small_struct(int x, int y, float z, char w) {
    struct SmallStruct s = {x, y, z, w};
    return s;
}

static struct MediumStruct make_medium_struct(double a, int b, short c, char d, float e) {
    struct MediumStruct m = {a, b, c, d, e};
    return m;
}

int main() {
    volatile int counter = 0;  /* Prevent constant propagation */
    int data1 = 42;
    int data2 = 100;
    short short_data = 32767;
    void* ptr = &data1;
    
    /* Array of results to prevent dead code elimination */
    double results[100];
    int result_idx = 0;
    
    /* Call 10-parameter functions in a loop */
    for (int i = 0; i < 10; i++) {
        /* Varying arguments to prevent optimization */
        int arg1 = counter + i;
        float arg2 = (float)(counter * 0.5 + i);
        double arg3 = (double)(counter * 0.25 + i);
        int* arg4 = (i % 2) ? &data1 : &data2;
        char arg5 = (char)(65 + i);
        short arg6 = (short)(short_data - i * 100);
        long arg7 = (long)(counter * 1000 + i);
        void* arg8 = (i % 3) ? ptr : (void*)&counter;
        size_t arg9 = (size_t)(counter * 100 + i);
        unsigned arg10 = (unsigned)(counter * 10 + i);
        
        /* Direct call to 10-parameter function */
        int res1 = func10(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
        
        /* Call through function pointer (obfuscates for compiler) */
        int res2 = func10_array[i % 2](arg1, arg2, arg3, arg4, arg5, arg6, 
                                      arg7, arg8, arg9, arg10);
        
        /* Call alternative 10-parameter function */
        long res3 = func10_alt(arg3, arg1, arg2, &short_data, arg7, arg10, 
                              arg9, arg5, arg1, arg8);
        
        results[result_idx++] = (double)(res1 + res2 + res3);
        
        /* Create structs for 11-parameter functions */
        struct SmallStruct s = make_small_struct(arg1, arg1 * 2, arg2, arg5);
        struct MediumStruct m = make_medium_struct(arg3, arg1, arg6, arg5, arg2);
        
        /* Direct call to 11-parameter function */
        double res4 = func11(arg1, arg2, arg3, arg4, arg5, arg6, arg7, 
                            arg8, arg9, arg10, s);
        
        /* Call through function pointer */
        double res5 = func11_array[i % 2](arg1, arg2, arg3, arg4, arg5, arg6,
                                         arg7, arg8, arg9, arg10, s);
        
        /* Call alternative 11-parameter function with struct as first parameter */
        float res6 = func11_alt(m, arg1, arg2, arg3, arg4, arg5, arg6, arg7,
                               arg8, arg9, arg10);
        
        results[result_idx++] = res4 + res5 + res6;
        
        counter++;
    }
    
    /* Conditional calls based on runtime values */
    if (counter > 5) {
        struct SmallStruct s = make_small_struct(1, 2, 3.0f, 'A');
        struct MediumStruct m = make_medium_struct(1.0, 2, 3, 'B', 4.0f);
        
        /* More calls with different argument patterns */
        for (int j = 0; j < 3; j++) {
            double r1 = func11(j, j*1.5f, j*2.5, &data1, 'X' + j, 
                              (short)j, j*100L, ptr, j*10, j*20, s);
            float r2 = func11_alt(m, j, j*2.0f, j*3.0, &data2, 'Y' + j,
                                 (short)(j*2), j*200L, &s, j*15, j*25);
            results[result_idx++] = r1 + r2;
        }
    }
    
    /* Sum results to prevent optimization and verify */
    double total = 0.0;
    for (int i = 0; i < result_idx; i++) {
        total += results[i];
    }
    
    printf("Total: %f\n", total);
    return (int)total % 256;
}
