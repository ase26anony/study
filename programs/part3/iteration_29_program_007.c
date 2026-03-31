#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
    int z;
    int w;
};

/* Mixed-type 10-parameter function */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex operations mixing types */
    double temp = c * b + a;
    if (d) *d += (int)temp;
    
    return result & 0xFF; /* Return something non-trivial */
}

/* Another 10-parameter function with struct parameter */
static inline int func10_struct(int a, struct Data16 s, double c, int* d, 
                                char e, short f, long g, void* h, size_t i) {
    /* Use struct members */
    volatile int result = a + s.a + s.b + (int)s.c + (int)s.d;
    result += (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i;
    
    /* Modify through pointer */
    if (d) *d += s.a * s.b;
    
    return result & 0xFF;
}

/* 11-parameter function */
static inline int func11(int a, float b, double c, int* d, char e,
                         short f, long g, void* h, size_t i, unsigned j,
                         struct Data32 s) {
    /* Use all 11 parameters */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Use struct members */
    result += (int)(s.x % 100) + (int)(s.y % 100) + s.z + s.w;
    
    /* Complex floating point operations */
    double temp = c * b * s.x;
    if (d) *d += (int)temp;
    
    /* Mix operations to prevent simplification */
    return (result * a) & 0xFF;
}

/* Another 11-parameter variant */
static inline double func11_mixed(float a, double b, int c, long d, 
                                  short e, char f, void* g, size_t h,
                                  unsigned i, int* j, struct Data16 s) {
    /* Mixed type computations */
    double result = (double)a * b * c * d;
    result += e * f * ((size_t)g % 100);
    result += h * i * s.a * s.b;
    
    /* Use pointer parameter */
    if (j) *j += (int)result;
    
    /* Return floating point to force different handling */
    return result / 1000.0;
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef int (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Data32);
typedef double (*Func11MixedPtr)(float, double, int, long, short, char, void*, size_t, unsigned, int*, struct Data16);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_struct};
static Func11Ptr func11_array[] = {func11};
static Func11MixedPtr func11_mixed_array[] = {func11_mixed};

/* Helper to create test data */
static struct Data16 make_data16(int seed) {
    struct Data16 d;
    d.a = seed * 2;
    d.b = seed * 3;
    d.c = seed * 1.5f;
    d.d = seed * 2.5f;
    return d;
}

static struct Data32 make_data32(int seed) {
    struct Data32 d;
    d.x = seed * 100LL;
    d.y = seed * 200LL;
    d.z = seed * 3;
    d.w = seed * 4;
    return d;
}

int main(void) {
    volatile int seed = 42; /* volatile to prevent constant propagation */
    int results[100];
    double fp_results[100];
    
    /* Test data */
    int int_val = 100;
    float float_val = 3.14f;
    double double_val = 2.71828;
    int ptr_val = 0;
    char char_val = 'A';
    short short_val = 32767;
    long long_val = 123456789L;
    void* ptr = &seed;
    size_t size_val = sizeof(struct Data16);
    unsigned uint_val = 4000000000U;
    
    /* Call 10-parameter functions in a loop */
    for (int i = 0; i < 50; i++) {
        /* Vary arguments to prevent optimization */
        int local_seed = seed + i;
        
        /* Call through function pointer (obfuscates for compiler) */
        int idx = local_seed % 2;
        results[i] = func10_array[idx](
            local_seed,
            float_val + i,
            double_val * i,
            &ptr_val,
            char_val + i,
            short_val - i,
            long_val + i,
            ptr,
            size_val + i,
            uint_val - i
        );
        
        /* Direct call to 10-parameter function with struct */
        if (i % 3 == 0) {
            struct Data16 s = make_data16(local_seed);
            results[i + 50] = func10_struct(
                local_seed * 2,
                s,  /* Struct passed by value */
                double_val / (i + 1),
                &ptr_val,
                char_val - i,
                short_val + i,
                long_val * 2,
                ptr,
                size_val * 2
            );
        }
    }
    
    /* Call 11-parameter functions */
    for (int i = 0; i < 25; i++) {
        int local_seed = seed * i;
        struct Data32 s = make_data32(local_seed);
        
        /* Call 11-parameter function */
        results[i + 75] = func11(
            local_seed,
            float_val * i,
            double_val + i,
            &ptr_val,
            char_val,
            short_val,
            long_val,
            ptr,
            size_val,
            uint_val,
            s  /* 11th parameter - struct by value */
        );
        
        /* Call mixed 11-parameter function */
        struct Data16 s2 = make_data16(local_seed);
        fp_results[i] = func11_mixed(
            float_val + i,
            double_val * 2,
            local_seed,
            long_val,
            short_val,
            char_val + i,
            ptr,
            size_val + i * 10,
            uint_val,
            &ptr_val,
            s2  /* 11th parameter */
        );
    }
    
    /* Call functions based on runtime input */
    volatile int choice = 0;
    for (int i = 0; i < 10; i++) {
        choice = results[i] % 3;
        
        struct Data32 s = make_data32(i * 10);
        struct Data16 s2 = make_data16(i * 5);
        
        switch (choice) {
            case 0:
                results[i] = func10(
                    i, float_val, double_val, &ptr_val,
                    'A' + i, 1000 + i, 10000L + i,
                    ptr, 256 + i, 1000 + i
                );
                break;
            case 1:
                results[i] = func11(
                    i * 2, float_val * 2, double_val * 3,
                    &ptr_val, 'B' + i, 2000 + i, 20000L + i,
                    ptr, 512 + i, 2000 + i, s
                );
                break;
            case 2:
                fp_results[i + 25] = func11_mixed(
                    float_val * 3, double_val * 4, i * 3,
                    30000L + i, 3000 + i, 'C' + i,
                    ptr, 1024 + i, 3000 + i, &ptr_val, s2
                );
                break;
        }
    }
    
    /* Prevent entire program from being optimized away */
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += results[i];
    }
    printf("Result: %d\n", sum);
    
    return 0;
}
