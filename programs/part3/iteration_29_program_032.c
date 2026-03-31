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
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile double g_volatile_double = 2.71828;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((uintptr_t)h % 256);
    sum += (int)i + j;
    
    /* Complex arithmetic mixing types */
    double result = (double)a * b + c / (*d + 1);
    result += (double)e * f + g;
    
    /* Return value depends on all parameters */
    return sum + (int)result;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(struct Data16 s1, int a, float b, double c, 
                           int* d, char e, short f, long g, void* h, 
                           size_t i, unsigned j) {
    /* Use all parameters including struct members */
    double total = s1.a + s1.b + s1.c + s1.d;
    total += a + b + c + *d + e + f + g;
    total += (double)((uintptr_t)h % 1024);
    total += i + j;
    
    /* Complex computation using all parameters */
    total = total * (s1.c + 1.0f) / (b + 1.0f);
    total += (double)(a * *d) / (g + 1);
    
    return total;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, short* d, 
                             struct Data24 s, char e, void* f, 
                             size_t g, unsigned h, int i) {
    /* Use struct members */
    long result = (long)a + b + (long)c + *d + s.y + e;
    result += (long)((uintptr_t)f % 512);
    result += g + h + i + (long)s.x;
    
    /* Complex operations */
    double temp = s.x * a + c * b;
    result += (long)temp;
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(int a, struct Data16 s1, float b, 
                              double c, int* d, struct Data24 s2,
                              char e, short f, long g, void* h, 
                              unsigned i) {
    float total = (float)a + s1.c + s1.d + b;
    total += (float)c + (float)*d + (float)s2.y + e + f + g;
    total += (float)((uintptr_t)h % 256) + i;
    
    /* Mix struct members */
    total = total * s1.c / (b + 1.0f);
    total += (float)(s2.x * c);
    
    return total;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(struct Data16, int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func10AltPtr)(double, int, float, short*, struct Data24, char, void*, size_t, unsigned, int);
typedef float (*Func11AltPtr)(int, struct Data16, float, double, int*, struct Data24, char, short, long, void*, unsigned);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

/* Helper to create test data */
static struct Data16 make_data16(int a, int b, float c, float d) {
    struct Data16 s = {a, b, c, d};
    return s;
}

static struct Data24 make_data24(double x, int y, short z, const char* w) {
    struct Data24 s = {x, y, z};
    for (int i = 0; i < 4 && w[i]; i++) {
        s.w[i] = w[i];
    }
    return s;
}

int main() {
    int int_array[5] = {1, 2, 3, 4, 5};
    short short_array[3] = {10, 20, 30};
    
    struct Data16 s1 = make_data16(100, 200, 3.5f, 4.5f);
    struct Data24 s2 = make_data24(99.9, 88, 77, "test");
    
    volatile int counter = g_volatile_int; /* Prevent optimization */
    double total_result = 0.0;
    
    /* Call functions multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        /* Mix of constant and volatile arguments */
        int result10 = func10(
            i,                          /* int */
            g_volatile_float + i,       /* float */
            g_volatile_double,          /* double */
            int_array,                  /* int* */
            'A' + (i % 26),             /* char */
            (short)(i * 2),             /* short */
            (long)i * 1000,             /* long */
            (void*)(uintptr_t)i,        /* void* */
            (size_t)(i * 10),           /* size_t */
            (unsigned)(i * 100)         /* unsigned */
        );
        
        struct Data16 dynamic_s1 = make_data16(i, i*2, i*3.0f, i*4.0f);
        double result11 = func11(
            dynamic_s1,                 /* struct Data16 by value */
            counter + i,                /* int */
            (float)i / 2.0f,            /* float */
            (double)i / 3.0,            /* double */
            &int_array[i % 5],          /* int* */
            'Z' - (i % 26),             /* char */
            (short)(100 - i),           /* short */
            (long)(1000 + i),           /* long */
            (void*)(uintptr_t)(i * 100),/* void* */
            (size_t)(500 + i),          /* size_t */
            (unsigned)(1000 + i)        /* unsigned */
        );
        
        /* Alternate functions */
        struct Data24 dynamic_s2 = make_data24(i * 1.5, i * 2, i * 3, "abcd");
        long result10_alt = func10_alt(
            (double)i / 7.0,            /* double */
            i * 3,                      /* int */
            (float)i / 11.0f,           /* float */
            short_array,                /* short* */
            dynamic_s2,                 /* struct Data24 by value */
            'M' + (i % 10),             /* char */
            (void*)(uintptr_t)(i * 50), /* void* */
            (size_t)(200 + i),          /* size_t */
            (unsigned)(300 + i),        /* unsigned */
            i * 4                       /* int */
        );
        
        float result11_alt = func11_alt(
            i * 5,                      /* int */
            s1,                         /* struct Data16 by value */
            (float)i / 13.0f,           /* float */
            (double)i / 17.0,           /* double */
            &int_array[(i + 1) % 5],    /* int* */
            s2,                         /* struct Data24 by value */
            'N' - (i % 10),             /* char */
            (short)(50 + i),            /* short */
            (long)(2000 + i),           /* long */
            (void*)(uintptr_t)(i * 75), /* void* */
            (unsigned)(1500 + i)        /* unsigned */
        );
        
        total_result += result10 + result11 + result10_alt + result11_alt;
        counter++; /* Change volatile variable */
    }
    
    /* Call through function pointers with volatile index */
    volatile int func_index = g_volatile_int % 2;
    
    for (int i = 0; i < 50; i++) {
        struct Data16 ptr_s1 = make_data16(i * 10, i * 20, i * 1.5f, i * 2.5f);
        
        /* Call 10-parameter function through pointer */
        int ptr_result10 = func10_array[func_index](
            i * 2,
            (float)i * 1.1f,
            (double)i * 1.2,
            int_array,
            'C' + (i % 20),
            (short)(i * 3),
            (long)(i * 100),
            (void*)(uintptr_t)(i * 200),
            (size_t)(i * 30),
            (unsigned)(i * 40)
        );
        
        /* Call 11-parameter function through pointer */
        double ptr_result11 = func11_array[func_index](
            ptr_s1,
            i * 3,
            (float)i * 2.1f,
            (double)i * 2.2,
            &int_array[i % 5],
            'D' + (i % 20),
            (short)(i * 4),
            (long)(i * 200),
            (void*)(uintptr_t)(i * 300),
            (size_t)(i * 50),
            (unsigned)(i * 60)
        );
        
        total_result += ptr_result10 + ptr_result11;
        func_index = (func_index + 1) % 2; /* Change index */
    }
    
    printf("Total result: %f\n", total_result);
    return (int)total_result % 256;
}
