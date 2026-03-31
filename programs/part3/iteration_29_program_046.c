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
    int z;
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
    
    /* Some arithmetic mixing types */
    float fsum = b * (float)c + (float)a;
    double dsum = c * (double)b + (double)g;
    
    /* Return value depends on all parameters */
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data32 s2, 
                           size_t i, unsigned j, void* h) {
    /* Use all parameters including structs */
    double result = (double)a + (double)s1.a + (double)s1.b + s1.c + s1.d;
    result += b + c + (double)*d + (double)e + (double)f;
    result += (double)s2.x + (double)s2.y + (double)s2.z + (double)s2.w;
    result += (double)i + (double)j + (double)((size_t)h % 256);
    
    /* Cross-type operations */
    result += (double)(a * s1.a) / (b + 1.0f);
    result += (double)(s2.x % 1000) * c;
    
    return result;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, void* d, short e,
                             long f, size_t g, unsigned h, int* i, char j) {
    long result = (long)a + b + (long)c + (long)d + e + f + (long)g + h;
    result += *i * 2 + (long)j * 3;
    
    /* Complex expression using all parameters */
    result += (long)((a * (double)b) / (c + 1.0f));
    result += (long)(((size_t)d) % 100) * f;
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(struct Data32 s1, int a, float b, double c,
                              int* d, char e, short f, long g, void* h,
                              size_t i, unsigned j) {
    float result = (float)s1.x + (float)s1.y + (float)s1.z + (float)s1.w;
    result += (float)a + b + (float)c + (float)*d + (float)e + (float)f;
    result += (float)g + (float)((size_t)h % 256) + (float)i + (float)j;
    
    /* Mixed operations */
    result += b * (float)c - (float)(a * *d);
    result += (float)(s1.x / 1000) * (float)(s1.y / 1000);
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, struct Data32, size_t, unsigned, void*);
typedef long (*Func10AltPtr)(double, int, float, void*, short, long, size_t, unsigned, int*, char);
typedef float (*Func11AltPtr)(struct Data32, int, float, double, int*, char, short, long, void*, size_t, unsigned);

/* Array of function pointers */
Func10Ptr func10_array[2] = {func10, (Func10Ptr)func10_alt};
Func11Ptr func11_array[2] = {func11, (Func11Ptr)func11_alt};

int main() {
    int data1 = 100;
    int data2 = 200;
    int* ptr1 = &data1;
    int* ptr2 = &data2;
    
    /* Initialize structs */
    struct Data16 s16 = {10, 20, 30.0f, 40.0f};
    struct Data32 s32 = {1000LL, 2000LL, 3000, 4000};
    struct Data16 s16_alt = {50, 60, 70.0f, 80.0f};
    struct Data32 s32_alt = {5000LL, 6000LL, 7000, 8000};
    
    int total = 0;
    double total_d = 0.0;
    
    /* Loop to call functions multiple times with varying arguments */
    for (int i = 0; i < 100; i++) {
        /* Use volatile variables to prevent constant propagation */
        int arg1 = v1 + i;
        float arg2 = v2 + (float)i;
        double arg3 = v3 + (double)i;
        char arg5 = v4 + (char)(i % 26);
        
        /* Call 10-parameter functions */
        if (i % 3 == 0) {
            total += func10(arg1, arg2, arg3, ptr1, arg5, 
                           (short)i, (long)i * 2, (void*)(size_t)i, 
                           (size_t)i * 3, (unsigned)i * 4);
        } else if (i % 3 == 1) {
            total += (int)func10_alt(arg3, arg1, arg2, (void*)(size_t)i,
                                    (short)i, (long)i, (size_t)i * 2,
                                    (unsigned)i * 3, ptr2, arg5);
        }
        
        /* Call 11-parameter functions */
        if (i % 4 == 0) {
            total_d += func11(arg1, s16, arg2, arg3, ptr1, arg5,
                             (short)i, s32, (size_t)i * 5,
                             (unsigned)i * 6, (void*)(size_t)i);
        } else if (i % 4 == 2) {
            total_d += (double)func11_alt(s32_alt, arg1, arg2, arg3, ptr2,
                                         arg5, (short)i, (long)i * 7,
                                         (void*)(size_t)i, (size_t)i * 8,
                                         (unsigned)i * 9);
        }
        
        /* Modify structs slightly each iteration */
        s16.a++;
        s16.b += 2;
        s16.c += 0.5f;
        s16.d += 0.25f;
        
        s32.x += 10;
        s32.y += 20;
        s32.z++;
        s32.w += 2;
        
        /* Switch pointers occasionally */
        if (i % 10 == 0) {
            int* temp = ptr1;
            ptr1 = ptr2;
            ptr2 = temp;
        }
    }
    
    /* Call through function pointer array */
    int selector = total % 4;
    
    if (selector < 2) {
        /* Call 10-parameter function through pointer */
        total += func10_array[selector](v1, v2, v3, ptr1, v4,
                                       (short)selector, (long)selector * 10,
                                       (void*)(size_t)selector, 
                                       (size_t)selector * 20,
                                       (unsigned)selector * 30);
    } else {
        /* Call 11-parameter function through pointer */
        total_d += func11_array[selector - 2](v1, s16_alt, v2, v3, ptr2,
                                             v4, (short)selector, s32_alt,
                                             (size_t)selector * 40,
                                             (unsigned)selector * 50,
                                             (void*)(size_t)selector);
    }
    
    printf("Results: total=%d, total_d=%.2f\n", total, total_d);
    return 0;
}
