#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value (forces complex parameter passing) */
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
    char w[5];
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
    if (h) sum += (int)((intptr_t)h & 0xFF);
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
    /* Use all parameters including struct fields */
    double sum = (double)a + s1.a + s1.b + s1.c + s1.d;
    sum += b + c;
    if (d) sum += *d;
    sum += e + f + s2.x + s2.y + s2.z;
    sum += i + j + k;
    
    /* Complex operations with struct fields */
    sum += s1.c * s2.x - s1.d / (s2.y + 1);
    
    return sum;
}

/* Another 10-parameter function with different signature */
static inline void* func10_ptr(void* p1, int a, float b, double c, 
                              struct Data16 s, int* d, char e, 
                              short f, long g, size_t h) {
    /* Use parameters meaningfully */
    int offset = a + (int)b + (int)c + s.a + s.b;
    if (d) offset += *d;
    offset += e + f + g + h;
    
    /* Return pointer with offset */
    return (char*)p1 + (offset % 256);
}

/* Another 11-parameter function */
static inline int func11_mixed(double a, int b, float c, struct Data24 s1,
                              void* p, short d, long e, int f, 
                              struct Data16 s2, unsigned g, char h) {
    int result = (int)a + b + (int)c + (int)s1.x + s1.y + s1.z;
    result += (int)((intptr_t)p & 0xFF);
    result += d + e + f + s2.a + s2.b + g + h;
    
    /* Complex floating point operations */
    double fp_result = a * c + s1.x * s2.c - s2.d;
    result += (int)fp_result;
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Type)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Type)(int, struct Data16, float, double, int*, char, short, struct Data24, size_t, unsigned, long);
typedef void* (*Func10PtrType)(void*, int, float, double, struct Data16, int*, char, short, long, size_t);

/* Array of function pointers to obfuscate calls */
static Func10Type func10_array[] = {func10, (Func10Type)func10_ptr};
static Func11Type func11_array[] = {func11, func11_mixed};

int main() {
    struct Data16 s1 = {10, 20, 30.5f, 40.5f};
    struct Data24 s2 = {100.5, 200, 300, "test"};
    int local_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    int total = 0;
    double total_d = 0.0;
    
    /* Loop with varying arguments to trigger different expansion paths */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and constant arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + i * 0.5f;
        double arg3 = v3 + i * 0.25;
        int* arg4 = (i % 3 == 0) ? &local_array[i % 10] : NULL;
        
        /* Call 10-parameter function directly */
        int result10 = func10(arg1, arg2, arg3, arg4,
                             (char)('A' + i % 26),
                             (short)(100 + i),
                             (long)(1000 + i * 2),
                             (void*)(intptr_t)(i * 100),
                             (size_t)(i * 10),
                             (unsigned)(i * 5));
        total += result10;
        
        /* Call 11-parameter function directly */
        double result11 = func11(arg1, s1, arg2, arg3, arg4,
                                (char)('a' + i % 26),
                                (short)(200 + i),
                                s2,
                                (size_t)(i * 20),
                                (unsigned)(i * 3),
                                (long)(500 + i));
        total_d += result11;
        
        /* Modify structs slightly each iteration */
        s1.a++;
        s1.c += 0.1f;
        s2.x += 0.5;
        s2.y++;
        
        /* Call through function pointer (prevents optimization) */
        if (i % 2 == 0) {
            Func10Type fp = func10_array[i % 2];
            total += fp(arg1, arg2, arg3, arg4,
                       (char)('Z' - i % 26),
                       (short)(50 + i),
                       (long)(2000 + i),
                       (void*)(intptr_t)(i * 50),
                       (size_t)(i * 15),
                       (unsigned)(i * 7));
        }
        
        /* Call 11-parameter through pointer */
        if (i % 3 == 0) {
            Func11Type fp = func11_array[i % 2];
            total_d += fp(arg1, s1, arg2, arg3, arg4,
                         (char)('m' + i % 13),
                         (short)(150 + i),
                         s2,
                         (size_t)(i * 25),
                         (unsigned)(i * 4),
                         (long)(750 + i));
        }
        
        /* Call the other 10-parameter function */
        void* ptr_result = func10_ptr((void*)(intptr_t)i, 
                                     arg1, arg2, arg3, s1,
                                     arg4,
                                     (char)('G' + i % 20),
                                     (short)(75 + i),
                                     (long)(1250 + i),
                                     (size_t)(i * 30));
        total += (int)(intptr_t)ptr_result;
    }
    
    printf("Results: total=%d, total_d=%.2f\n", total, total_d);
    return 0;
}
