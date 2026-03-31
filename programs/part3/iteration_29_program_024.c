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

struct Data24 {
    double x;
    int y;
    short z;
    char w[6];
};

/* Mixed-type functions with exactly 10 parameters */
static inline int func10_1(int a, float b, double c, int* d, char e, 
                          short f, long g, void* h, size_t i, unsigned j) {
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    return result;
}

static inline double func10_2(struct Data16 s1, int a, float b, double c, 
                             int* d, char e, short f, long g, void* h) {
    volatile double result = s1.a + s1.b + s1.c + s1.d + a + b + c;
    result += *d + e + f + g + (double)((size_t)h);
    return result;
}

/* Mixed-type functions with exactly 11 parameters */
static inline long func11_1(int a, float b, double c, int* d, char e,
                           short f, long g, void* h, size_t i, 
                           unsigned j, struct Data24 s2) {
    volatile long result = a + (long)b + (long)c + *d + e + f + g;
    result += (long)((size_t)h) + i + j + (long)s2.x + s2.y;
    return result;
}

static inline float func11_2(struct Data16 s1, int a, float b, double c,
                            int* d, char e, short f, long g, void* h,
                            size_t i, unsigned j) {
    volatile float result = s1.a + s1.b + s1.c + s1.d + a + b + (float)c;
    result += *d + e + f + (float)g + (float)((size_t)h) + i + j;
    return result;
}

/* Function pointer types */
typedef int (*func10_ptr_t)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*func10_ptr2_t)(struct Data16, int, float, double, int*, char, short, long, void*);
typedef long (*func11_ptr_t)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Data24);
typedef float (*func11_ptr2_t)(struct Data16, int, float, double, int*, char, short, long, void*, size_t, unsigned);

/* Function pointer array to obfuscate calls */
static volatile func10_ptr_t func10_array[2] = {func10_1, (func10_ptr_t)func10_2};
static volatile func11_ptr_t func11_array[2] = {func11_1, (func11_ptr_t)func11_2};

int main(void) {
    volatile int seed = 42; /* Prevent constant propagation */
    int data1 = 100;
    int data2 = 200;
    int data3 = 300;
    
    struct Data16 s1 = {1, 2, 3.5f, 4.5f};
    struct Data24 s2 = {5.5, 6, 7, "hello"};
    
    int total = 0;
    double total_d = 0.0;
    long total_l = 0;
    float total_f = 0.0f;
    
    /* Call functions multiple times with varying arguments */
    for (int i = 0; i < 100; i++) {
        /* Vary parameters to prevent optimization */
        int a = seed + i;
        float b = (float)(seed * i) / 3.14f;
        double c = (double)(seed - i) * 2.71828;
        char e = (char)(i % 256);
        short f = (short)(i * 2);
        long g = (long)(i * 1000);
        size_t sz = (size_t)(i * 100);
        unsigned u = (unsigned)(i * 50);
        
        /* Call 10-parameter functions */
        if (i % 3 == 0) {
            total += func10_1(a, b, c, &data1, e, f, g, (void*)(size_t)i, sz, u);
            
            /* Update struct to vary it */
            s1.a = i;
            s1.c = (float)i / 10.0f;
            total_d += func10_2(s1, a, b, c, &data2, e, f, g, (void*)(size_t)(i + 1));
        }
        
        /* Call 11-parameter functions */
        if (i % 4 == 0) {
            /* Update second struct */
            s2.x = (double)i / 3.0;
            s2.y = i * 2;
            total_l += func11_1(a, b, c, &data3, e, f, g, (void*)(size_t)i, sz, u, s2);
            
            total_f += func11_2(s1, a, b, c, &data1, e, f, g, (void*)(size_t)i, sz, u);
        }
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (i % 5 == 0) {
            int idx = i % 2;
            total += func10_array[idx](a, b, c, &data1, e, f, g, (void*)(size_t)i, sz, u);
            
            /* Need to cast for the struct version */
            if (idx == 1) {
                total_d += ((func10_ptr2_t)func10_array[idx])(s1, a, b, c, &data2, e, f, g, (void*)(size_t)i);
            }
        }
        
        if (i % 7 == 0) {
            int idx = i % 2;
            total_l += func11_array[idx](a, b, c, &data3, e, f, g, (void*)(size_t)i, sz, u, s2);
        }
        
        /* Create cross-function optimization opportunities */
        if (i < 10) {
            /* Call with constants to allow constant propagation */
            total += func10_1(1, 2.0f, 3.0, &data1, 'a', 10, 100L, NULL, 1000, 2000);
            total_l += func11_1(1, 2.0f, 3.0, &data1, 'a', 10, 100L, NULL, 1000, 2000, s2);
        }
    }
    
    printf("Results: %d, %f, %ld, %f\n", total, total_d, total_l, total_f);
    return 0;
}
