#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value (forces complex parameter passing) */
struct Data16 {
    int a;
    float b;
    short c;
    char d;
};

struct Data32 {
    double x;
    int y;
    float z;
    char w[8];
};

/* Mixed parameter types to prevent optimization */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    return result;
}

static inline double func11(double a, int b, float c, void* d, short e,
                            long f, size_t g, char h, unsigned i, 
                            struct Data16 s, int* j) {
    /* Complex usage of all parameters */
    double result = a + b + c + (double)((size_t)d % 100);
    result += e + f + g + h + i + s.a + s.b + *j;
    return result;
}

/* Another 10-parameter function with struct parameter */
static inline long func10_struct(int a, struct Data32 s1, float b, 
                                 double c, int* d, char e, short f, 
                                 void* h, size_t i, unsigned j) {
    long result = a + (long)s1.x + (long)b + (long)c + *d;
    result += e + f + (long)((size_t)h) + i + j + s1.y;
    return result;
}

/* Another 11-parameter function */
static inline float func11_mixed(struct Data16 s1, int a, float b, 
                                 double c, int* d, char e, short f,
                                 long g, void* h, size_t i, unsigned j) {
    float result = s1.a + s1.b + a + b + (float)c + *d;
    result += e + f + g + (float)((size_t)h) + i + j;
    return result;
}

/* Function pointer types to obfuscate calls */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, 
                         long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(double, int, float, void*, short,
                            long, size_t, char, unsigned,
                            struct Data16, int*);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_struct};
static Func11Ptr func11_array[] = {func11, func11_mixed};

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile double g_volatile_double = 2.71828;

int main() {
    int data1 = 100;
    int data2 = 200;
    int data3 = 300;
    
    struct Data16 s16 = {10, 20.5f, 30, 'A'};
    struct Data32 s32 = {3.14159, 42, 2.718f, "test123"};
    
    volatile int selector = 0;
    
    /* Call 10-parameter functions in a loop */
    long total10 = 0;
    for (int i = 0; i < 100; i++) {
        /* Vary arguments to prevent optimization */
        int arg1 = g_volatile_int + i;
        float arg2 = g_volatile_float + i * 0.1f;
        double arg3 = g_volatile_double + i * 0.01;
        
        /* Call through function pointer with volatile guard */
        if (selector++ % 2 == 0) {
            total10 += func10(arg1, arg2, arg3, &data1, 
                             'A' + (i % 26), i, i * 100L,
                             (void*)(size_t)i, i * 2, i * 3);
        } else {
            total10 += func10_struct(arg1, s32, arg2, arg3, &data2,
                                    'B' + (i % 26), i * 2,
                                    (void*)(size_t)(i + 1), i * 3, i * 4);
        }
    }
    
    /* Call 11-parameter functions */
    double total11 = 0.0;
    for (int i = 0; i < 50; i++) {
        double arg1 = g_volatile_double + i * 0.5;
        float arg2 = g_volatile_float + i * 0.2f;
        
        /* Alternate between two 11-parameter functions */
        if (i % 3 == 0) {
            total11 += func11(arg1, g_volatile_int + i, arg2,
                             (void*)(size_t)(data3 + i), i,
                             i * 50L, i * 10, 'X' + (i % 10),
                             i * 20, s16, &data3);
        } else {
            total11 += func11_mixed(s16, g_volatile_int + i * 2, arg2,
                                   arg1, &data1, 'Y' + (i % 10),
                                   i * 3, i * 75L,
                                   (void*)(size_t)(data2 + i),
                                   i * 15, i * 30);
        }
    }
    
    /* Use function pointer arrays */
    int idx = g_volatile_int % 2;
    int result10 = func10_array[idx](100, 200.5f, 300.75, &data1,
                                    'Z', 500, 600L, 
                                    (void*)0x1000, 700, 800);
    
    idx = (g_volatile_int + 1) % 2;
    double result11 = func11_array[idx](400.25, 500, 600.5f,
                                       (void*)0x2000, 700,
                                       800L, 900, 'W',
                                       1000, s16, &data2);
    
    printf("Results: total10=%ld, total11=%f, result10=%d, result11=%f\n",
           total10, total11, result10, result11);
    
    return 0;
}
