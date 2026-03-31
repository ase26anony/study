#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

/* Small structs to pass by value (16-32 bytes) */
struct Data16 {
    int a;
    float b;
    double c;
    char d;
};

struct Data32 {
    long long x;
    double y;
    int z;
    float w;
    char pad[4];
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
    sum += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex arithmetic mixing types */
    float fsum = b * (float)c + (float)a;
    double dsum = c * (double)b + (double)g;
    
    /* Pointer arithmetic */
    if (d) *d += sum;
    
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(struct Data16 s1, int a, float b, double c, 
                           int* d, char e, short f, long g, void* h, 
                           size_t i, unsigned j) {
    /* Use all parameters including struct members */
    double total = s1.a + s1.b + s1.c + s1.d;
    total += a + b + c + *d + e + f + g;
    total += (double)((size_t)h % 256) + i + j;
    
    /* Complex operations */
    total *= (b > 0.0f) ? 1.1 : 0.9;
    total += (c * (double)g) / (a + 1);
    
    if (d) *d += (int)total;
    
    return total;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, float b, int c, struct Data32 s,
                             char* d, short e, long f, size_t g, 
                             unsigned h, int* out) {
    /* Use struct members */
    long result = (long)s.x + (long)s.y + s.z;
    result += (long)a + (long)b + c + (long)(*d) + e + f + (long)g + h;
    
    /* Store result through pointer */
    if (out) *out = (int)result;
    
    return result * (f > 0 ? 2 : 1);
}

/* Another 11-parameter function */
static inline float func11_alt(int a, struct Data16 s1, float b, double c,
                              struct Data32 s2, char* d, short e, long f,
                              size_t g, unsigned h, int* out) {
    float result = (float)a + s1.b + s2.w + b;
    result += (float)c + (float)(*d) + (float)e + (float)f + (float)g + (float)h;
    
    /* Complex floating point operations */
    result = result * (s1.c > 0.0 ? 1.5f : 0.5f);
    result += (s2.y * s2.z) / (a + 1.0f);
    
    if (out) *out = (int)result;
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(struct Data16, int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func10AltPtr)(double, float, int, struct Data32, char*, short, long, size_t, unsigned, int*);
typedef float (*Func11AltPtr)(int, struct Data16, float, double, struct Data32, char*, short, long, size_t, unsigned, int*);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, NULL};
static Func11Ptr func11_array[] = {func11, NULL};
static Func10AltPtr func10_alt_array[] = {func10_alt, NULL};
static Func11AltPtr func11_alt_array[] = {func11_alt, NULL};

int main(int argc, char** argv) {
    int result1, result2;
    long result3;
    float result4;
    double result5;
    
    /* Local variables that will be used as arguments */
    int x = 10;
    float y = 20.5f;
    double z = 30.75;
    int data = 100;
    char ch = 'X';
    short sh = 500;
    long lg = 1000L;
    void* ptr = &data;
    size_t sz = 1024;
    unsigned un = 2048;
    
    /* Initialize structs */
    struct Data16 s1 = {5, 10.5f, 15.75, 'S'};
    struct Data32 s2 = {1000LL, 2000.0, 3000, 4000.0f, "pad"};
    
    /* Call functions directly in a loop - mixing constant and volatile args */
    for (int i = 0; i < 100; i++) {
        /* Call 10-parameter function */
        result1 = func10(
            v1 + i,        /* int - volatile based */
            y + i,         /* float */
            z * 2.0,       /* double */
            &data,         /* int* */
            ch + i,        /* char */
            sh,            /* short */
            lg * i,        /* long */
            ptr,           /* void* */
            sz + i,        /* size_t */
            un             /* unsigned */
        );
        
        /* Call 11-parameter function with struct by value */
        s1.a = i;
        s1.b = y + i * 0.5f;
        result5 = func11(
            s1,            /* struct Data16 by value */
            x + i,         /* int */
            v2 * i,        /* float - volatile based */
            z,             /* double */
            &data,         /* int* */
            v4 + i,        /* char - volatile based */
            sh + i,        /* short */
            lg,            /* long */
            ptr,           /* void* */
            sz,            /* size_t */
            un + i         /* unsigned */
        );
        
        /* Call alternate 10-parameter function */
        s2.x = i * 100LL;
        result3 = func10_alt(
            v3 + i,        /* double - volatile based */
            y,             /* float */
            x * i,         /* int */
            s2,            /* struct Data32 by value */
            &ch,           /* char* */
            sh,            /* short */
            lg + i,        /* long */
            sz * 2,        /* size_t */
            un,            /* unsigned */
            &result2       /* int* */
        );
        
        /* Call alternate 11-parameter function */
        result4 = func11_alt(
            v1 * i,        /* int - volatile based */
            s1,            /* struct Data16 by value */
            y * 0.5f,      /* float */
            z + i,         /* double */
            s2,            /* struct Data32 by value */
            &ch,           /* char* */
            sh + i * 2,    /* short */
            lg,            /* long */
            sz + i * 10,   /* size_t */
            un,            /* unsigned */
            &result2       /* int* */
        );
        
        /* Use results to prevent elimination */
        if (result1 > 1000 || result5 > 2000.0 || result3 > 3000 || result4 > 4000.0f) {
            data += 1;
        }
    }
    
    /* Call through function pointer array based on runtime condition */
    int choice = (argc > 1) ? atoi(argv[1]) % 4 : 0;
    
    switch (choice) {
        case 0:
            if (func10_array[0]) {
                result1 = func10_array[0](x, y, z, &data, ch, sh, lg, ptr, sz, un);
            }
            break;
        case 1:
            if (func11_array[0]) {
                result5 = func11_array[0](s1, x, y, z, &data, ch, sh, lg, ptr, sz, un);
            }
            break;
        case 2:
            if (func10_alt_array[0]) {
                result3 = func10_alt_array[0](z, y, x, s2, &ch, sh, lg, sz, un, &result2);
            }
            break;
        case 3:
            if (func11_alt_array[0]) {
                result4 = func11_alt_array[0](x, s1, y, z, s2, &ch, sh, lg, sz, un, &result2);
            }
            break;
    }
    
    printf("Results: %d %ld %.2f %.2f\n", result1, result3, result4, result5);
    printf("Final data: %d\n", data);
    
    return 0;
}
