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

/* Function with exactly 10 parameters */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((size_t)h % 256);  /* Use pointer as integer */
    sum += (int)i + (int)j;
    
    /* Mixed type operations */
    float fsum = b * c + (float)a;
    double dsum = c * (double)b + (double)g;
    
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters */
static inline double func11(int a, double b, float c, struct Data16 d, 
                           int* e, short f, long g, void* h, 
                           size_t i, unsigned j, struct Data32 k) {
    /* Use all parameters extensively */
    double total = (double)a + b + (double)c;
    total += (double)d.a + (double)d.b + (double)d.c + (double)d.d;
    total += (double)*e + (double)f + (double)g;
    total += (double)((size_t)h % 65536);
    total += (double)i + (double)j;
    total += (double)k.x + (double)k.y + k.z + (double)k.w;
    
    /* Complex mixed operations */
    total *= 1.0 + (b / (c + 1.0f));
    total += (k.x > k.y) ? (double)d.a : (double)d.b;
    
    return total;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(float a, int b, double c, void* d, 
                             short e, long f, int* g, char h, 
                             size_t i, unsigned short j) {
    long result = (long)a + b + (long)c;
    result += (long)((size_t)d >> 4);
    result += e + f + *g + h + (long)i + j;
    
    /* Prevent optimization with side-effect simulation */
    if (g != NULL) {
        result += *g * 2;
    }
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(struct Data16 a, int b, double c, 
                              float d, int* e, short f, long g, 
                              void* h, size_t i, unsigned j, 
                              struct Data32 k) {
    float result = (float)a.a + (float)a.b + a.c + a.d;
    result += (float)b + (float)c + d;
    result += (float)*e + (float)f + (float)g;
    result += (float)((size_t)h % 1024);
    result += (float)i + (float)j;
    result += (float)k.x + (float)k.y + (float)k.z + (float)k.w;
    
    /* Conditional operations using parameters */
    result *= (b > 0) ? 1.1f : 0.9f;
    result += (k.z > 0.0) ? d : -d;
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, double, float, struct Data16, int*, short, long, void*, size_t, unsigned, struct Data32);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

int main() {
    int data1 = 100;
    int data2 = 200;
    int* ptr1 = &data1;
    int* ptr2 = &data2;
    
    struct Data16 d16 = {10, 20, 30.5f, 40.5f};
    struct Data16 d16_alt = {50, 60, 70.5f, 80.5f};
    
    struct Data32 d32 = {1000LL, 2000LL, 3000.0, 4000};
    struct Data32 d32_alt = {5000LL, 6000LL, 7000.0, 8000};
    
    int total = 0;
    double dtotal = 0.0;
    
    /* Loop with varying arguments to trigger different expansion paths */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and constant arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + (float)i;
        double arg3 = v3 + (double)i;
        char arg5 = v4 + (char)(i % 26);
        
        /* Call 10-parameter functions directly */
        total += func10(arg1, arg2, arg3, ptr1, arg5, 
                       (short)i, (long)i * 2, (void*)(size_t)i, 
                       (size_t)i * 3, (unsigned)i * 4);
        
        total += func10_alt(arg2, arg1, arg3, (void*)(size_t)i, 
                          (short)(i + 1), (long)i, ptr2, 
                          (char)(arg5 + 1), (size_t)i * 5, 
                          (unsigned short)(i * 6));
        
        /* Call 11-parameter functions directly */
        dtotal += func11(arg1, arg3, arg2, 
                        (i % 2) ? d16 : d16_alt,
                        ptr1, (short)i, (long)i, 
                        (void*)(size_t)(i + 100), 
                        (size_t)i * 7, (unsigned)i * 8,
                        (i % 3) ? d32 : d32_alt);
        
        dtotal += func11_alt((i % 2) ? d16 : d16_alt, arg1, arg3, arg2,
                           ptr2, (short)(i + 2), (long)(i * 3),
                           (void*)(size_t)(i + 200), 
                           (size_t)i * 9, (unsigned)i * 10,
                           (i % 3) ? d32 : d32_alt);
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (i % 5 == 0) {
            total += func10_array[i % 2](arg1, arg2, arg3, ptr1, arg5,
                                       (short)i, (long)i, (void*)(size_t)i,
                                       (size_t)i, (unsigned)i);
        }
        
        if (i % 7 == 0) {
            dtotal += func11_array[i % 2](arg1, arg3, arg2, d16, ptr2,
                                        (short)i, (long)i, (void*)(size_t)i,
                                        (size_t)i, (unsigned)i, d32);
        }
        
        /* Modify structs to create variation */
        d16.a += i;
        d16_alt.b += i;
        d32.x += i;
        d32_alt.y += i;
    }
    
    printf("Result: total = %d, dtotal = %f\n", total, dtotal);
    return 0;
}
