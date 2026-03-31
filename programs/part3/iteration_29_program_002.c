#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

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

/* Mixed-type 10-parameter function */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent elimination */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (size_t)h % 256;  /* Use pointer as integer */
    result += i + j;
    return result;
}

/* Another 10-parameter function with struct parameter */
static inline double func10_struct(int a, struct Data16 s1, float b, 
                                   double c, int* d, char e, short f, 
                                   void* h, size_t i, unsigned j) {
    volatile double result = a + s1.a + s1.b + s1.c + s1.d + b + c + *d;
    result += e + f + (size_t)h;
    result += i + j;
    return result;
}

/* 11-parameter function */
static inline long func11(int a, float b, double c, int* d, char e,
                          short f, long g, void* h, size_t i, unsigned j,
                          struct Data32 s2) {
    volatile long result = a + (long)b + (long)c + *d + e + f + g;
    result += (long)((size_t)h % 65536);
    result += i + j + s2.x + s2.y + (long)s2.z + s2.w;
    return result;
}

/* Another 11-parameter function with different type mix */
static inline void* func11_mixed(struct Data16 s1, int a, double b, 
                                 float c, int* d, long e, short f,
                                 char g, size_t h, unsigned i, void* j) {
    volatile size_t calc = s1.a + s1.b + a + (int)b + (int)c + *d;
    calc += e + f + g + h + i + (size_t)j;
    return (void*)(calc % 1024);
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func10StructPtr)(int, struct Data16, float, double, int*, char, short, void*, size_t, unsigned);
typedef long (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Data32);
typedef void* (*Func11MixedPtr)(struct Data16, int, double, float, int*, long, short, char, size_t, unsigned, void*);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_struct};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_mixed};

/* Helper to generate varying arguments */
static void generate_args(volatile int* counter, 
                         int* a, float* b, double* c, int** d,
                         char* e, short* f, long* g, void** h,
                         size_t* i, unsigned* j,
                         struct Data16* s1, struct Data32* s2) {
    *a = (*counter)++;
    *b = (*counter) * 0.5f;
    *c = (*counter) * 0.25;
    static int dummy_int = 42;
    *d = &dummy_int;
    *e = (*counter) % 128;
    *f = (*counter) % 32767;
    *g = (*counter) * 100L;
    *h = (void*)(size_t)(*counter);
    *i = (size_t)(*counter) * 2;
    *j = (unsigned)(*counter) * 3;
    
    s1->a = (*counter) + 1;
    s1->b = (*counter) + 2;
    s1->c = (*counter) * 0.75f;
    s1->d = (*counter) * 1.25f;
    
    s2->x = (*counter) * 10LL;
    s2->y = (*counter) * 20LL;
    s2->z = (*counter) * 0.125;
    s2->w = (*counter) + 100;
}

int main(void) {
    volatile int counter = 0;
    int total = 0;
    
    /* Local variables for arguments */
    int a; float b; double c; int* d; char e; short f; long g; 
    void* h; size_t i; unsigned j;
    struct Data16 s1;
    struct Data32 s2;
    
    /* Loop with multiple calls to high-arity functions */
    for (int iter = 0; iter < 1000; iter++) {
        generate_args(&counter, &a, &b, &c, &d, &e, &f, &g, &h, &i, &j, &s1, &s2);
        
        /* Call 10-parameter functions directly */
        int res1 = func10(a, b, c, d, e, f, g, h, i, j);
        double res2 = func10_struct(a, s1, b, c, d, e, f, h, i, j);
        
        /* Call 11-parameter functions directly */
        long res3 = func11(a, b, c, d, e, f, g, h, i, j, s2);
        void* res4 = func11_mixed(s1, a, c, b, d, g, f, e, i, j, h);
        
        /* Call through function pointers (prevents optimization) */
        int idx = counter % 2;
        int res5 = func10_array[idx](a, b, c, d, e, f, g, h, i, j);
        long res6 = func11_array[idx](a, b, c, d, e, f, g, h, i, j, s2);
        
        /* Use results to prevent dead code elimination */
        total += res1 + (int)res2 + (int)res3 + (int)(size_t)res4 + res5 + (int)res6;
        
        /* Prevent loop unrolling from simplifying parameter passing */
        if (total > 1000000) {
            total %= 1000000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
