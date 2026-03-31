#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct SmallStruct {
    int x;
    float y;
    double z;
    char w;
};

struct MediumStruct {
    long a;
    double b;
    int c;
    float d;
    short e;
};

/* Mixed parameter types to prevent optimization */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex arithmetic mixing types */
    *d = result * (int)(b * c) + (e << 2) - f;
    
    /* Return value depends on all parameters */
    return result % 256;
}

static inline double func11(double a, int b, float c, long* d, short e,
                           unsigned f, size_t g, char h, void* i, int j, float k) {
    /* Use all 11 parameters */
    double result = a + b + c + *d + e + f + g + h + j + k;
    
    /* Complex operations */
    *d = (long)(result * (b + c) * (f % 100));
    
    /* Pointer arithmetic */
    if (i != NULL) {
        result += *(double*)i;
    }
    
    return result;
}

/* Functions with struct parameters by value */
static inline struct SmallStruct func10_struct(int a, struct SmallStruct s1, 
                                              float b, double c, int* d,
                                              char e, short f, struct MediumStruct s2,
                                              size_t i, unsigned j) {
    /* Use all parameters including structs */
    s1.x += a + (int)b + (int)c + *d + e + f;
    s1.y += s2.d + j;
    s1.z += (double)s2.a + (double)s2.c;
    s1.w += (char)(i % 256);
    
    *d = s1.x * s2.c + (int)s1.y;
    
    return s1;
}

static inline int func11_mixed(struct MediumStruct s1, int a, float b, 
                              double c, int* d, char e, short f,
                              struct SmallStruct s2, size_t i, 
                              unsigned j, long k) {
    /* Use all 11 parameters including two structs by value */
    int result = s1.c + a + (int)b + (int)c + *d + e + f + s2.x + (int)i + j + (int)k;
    
    /* Complex struct manipulation */
    s1.a = result * k;
    s2.y = b * c;
    
    *d = result + s1.c - s2.x;
    
    return result;
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(double, int, float, long*, short, unsigned, size_t, char, void*, int, float);
typedef struct SmallStruct (*Func10StructPtr)(int, struct SmallStruct, float, double, int*, char, short, struct MediumStruct, size_t, unsigned);
typedef int (*Func11MixedPtr)(struct MediumStruct, int, float, double, int*, char, short, struct SmallStruct, size_t, unsigned, long);

/* Array of function pointers to prevent direct optimization */
static Func10Ptr func10_array[] = {func10, NULL};
static Func11Ptr func11_array[] = {func11, NULL};
static Func10StructPtr func10_struct_array[] = {func10_struct, NULL};
static Func11MixedPtr func11_mixed_array[] = {func11_mixed, NULL};

/* Volatile variables to prevent constant propagation */
static volatile int volatile_seed = 42;
static volatile float volatile_float = 3.14f;
static volatile double volatile_double = 2.71828;

int main() {
    int data1 = 100;
    int data2 = 200;
    long data3 = 300;
    double data4 = 400.5;
    
    struct SmallStruct s1 = {10, 20.5f, 30.75, 'A'};
    struct MediumStruct s2 = {1000, 2000.5, 3000, 4000.25f, 5000};
    
    int total = 0;
    
    /* Loop to create multiple call sites with varying arguments */
    for (int i = 0; i < 100; i++) {
        /* Call func10 with mixed arguments */
        int r1 = func10(
            i + volatile_seed,           /* int */
            volatile_float + i,          /* float */
            volatile_double * i,         /* double */
            &data1,                      /* int* */
            'A' + (i % 26),              /* char */
            i * 2,                       /* short */
            i * 1000L,                   /* long */
            (void*)&data4,               /* void* */
            i * sizeof(int),             /* size_t */
            i * 3                        /* unsigned */
        );
        
        /* Call func11 with mixed arguments */
        double r2 = func11(
            volatile_double + i,         /* double */
            i * 2 + volatile_seed,       /* int */
            volatile_float * 2,          /* float */
            &data3,                      /* long* */
            i % 100,                     /* short */
            i * 5,                       /* unsigned */
            i * sizeof(double),          /* size_t */
            'Z' - (i % 26),              /* char */
            i % 2 ? (void*)&data4 : NULL,/* void* */
            i * 7,                       /* int */
            volatile_float / (i + 1)     /* float */
        );
        
        /* Call struct functions */
        struct SmallStruct s3 = func10_struct(
            i,
            s1,
            volatile_float,
            volatile_double,
            &data2,
            'M' + (i % 10),
            i * 3,
            s2,
            i * 100,
            i * 2
        );
        
        int r3 = func11_mixed(
            s2,
            i,
            volatile_float,
            volatile_double,
            &data1,
            'N' + (i % 10),
            i * 4,
            s1,
            i * 50,
            i * 6,
            i * 1000L
        );
        
        /* Use function pointers to obfuscate calls */
        if (func10_array[0] != NULL) {
            int r4 = func10_array[0](
                r1, volatile_float, r2, &data2, 
                s3.w, (short)r3, data3, &s3,
                sizeof(s3), r3
            );
            total += r4;
        }
        
        if (func11_array[0] != NULL && (i % 3) == 0) {
            double r5 = func11_array[0](
                r2, r1, s3.y, &data3,
                (short)s3.x, r3, sizeof(s2),
                s3.w, &s1, data1, volatile_float
            );
            total += (int)r5;
        }
        
        total += r1 + (int)r2 + s3.x + r3;
        
        /* Modify structs for next iteration */
        s1.x++;
        s1.y += 0.5f;
        s2.c += i;
    }
    
    printf("Result: %d\n", total);
    printf("Data1: %d, Data2: %d, Data3: %ld\n", data1, data2, data3);
    
    return total > 0 ? 0 : 1;
}
