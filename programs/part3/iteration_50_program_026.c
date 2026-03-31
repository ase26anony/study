/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Mixed data types for alignment testing */
struct S {
    int a;
    float b;
    double c;
    char d;
    short e;
    long f;
};

/* __attribute__((noinline)) helper to create another context */
__attribute__((noinline))
static int process_subset(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Pattern 1: Simple post-increment */
    while (p < end) {
        sum += *p++;
    }
    
    /* Pattern 2: Post-decrement with reset */
    p = end - 1;
    while (p >= arr) {
        sum += *p--;
    }
    
    return sum;
}

/* Another noinline helper with struct access */
__attribute__((noinline))
static float process_structs(struct S* arr, int n) {
    float total = 0.0f;
    struct S* ptr = arr;
    struct S* end = arr + n;
    
    /* Access different struct members with zero offset for first field */
    while (ptr < end) {
        total += ptr->a;      /* Base + 0 offset */
        total += ptr->b;      /* Base + 4 offset (assuming 4-byte int) */
        total += (float)ptr->c; /* Base + 8 offset (assuming 8-byte alignment) */
        ptr++;
    }
    
    return total;
}

int main(void) {
    /* Declare arrays of different types and sizes */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    char arr_c[200];
    short arr_s[200];
    struct S arr_struct[100];
    
    int i, j;
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Initialize with random values */
    srand(42);
    for (i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
        arr_c[i] = (char)(rand() % 256);
        arr_s[i] = (short)(rand() % 1000);
    }
    
    for (i = 0; i < 100; i++) {
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].c = (double)(rand() % 1000) / 10.0;
        arr_struct[i].d = (char)(rand() % 256);
        arr_struct[i].e = (short)(rand() % 1000);
        arr_struct[i].f = (long)(rand() % 1000);
    }
    
    /* ===== PATTERN 1: Simple post-increment with int pointer ===== */
    {
        register int* p __asm__("r12") = arr_i; /* Hint for register allocation */
        int* end = arr_i + 100;
        
        #pragma GCC unroll 4
        for (; p < end; ) {
            sum += *p++;  /* Should trigger find_inc with [reg + 0] */
        }
        
        /* Function call to force re-evaluation */
        fsum += sinf((float)sum);
    }
    
    /* ===== PATTERN 2: Post-decrement with float pointer ===== */
    {
        float* q = arr_f + 100;
        float* start = arr_f;
        
        while (q > start) {
            fsum += *--q;  /* Pre-decrement in access */
        }
        
        /* Reset and use post-decrement */
        q = arr_f + 99;
        while (q >= arr_f) {
            fsum += *q--;
        }
        
        /* Compiler barrier via math function */
        dsum += cos((double)fsum);
    }
    
    /* ===== PATTERN 3: Nested loops with pointer reset ===== */
    {
        int* base_ptr = arr_i;
        
        for (j = 0; j < 4; j++) {
            int* p = base_ptr + j * 25;
            int* end = p + 25;
            
            /* Inner loop with pointer increment */
            while (p < end) {
                sum += *p++;
            }
            
            /* Function call between pointer operations */
            if (j % 2 == 0) {
                fsum += sqrtf((float)sum);
            }
        }
    }
    
    /* ===== PATTERN 4: Mixed integer index and pointer arithmetic ===== */
    {
        int* __restrict r1 = arr_i + 50;  /* restrict helps alias analysis */
        int* __restrict r2 = arr_i + 150;
        
        for (i = 0; i < 50; i++) {
            /* Different pointer variables with zero offset */
            sum += *(r1 + i);  /* [reg + 0] when i=0 */
            sum += *(r2 + i);
            
            /* Compound expression with index modification */
            sum += arr_i[i++];  /* Post-increment in array index */
            sum += arr_i[--i];  /* Pre-decrement to reset */
        }
        
        /* Inline assembly to clobber memory */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* ===== PATTERN 5: Struct array traversal ===== */
    {
        struct S* ptr = arr_struct;
        struct S* end = arr_struct + 100;
        
        /* Loop accessing struct members with different offsets */
        while (ptr < end) {
            /* These decompose to base + constant_offset */
            sum += ptr->a;      /* offset 0 */
            fsum += ptr->b;     /* offset 4 */
            dsum += ptr->c;     /* offset 8 */
            
            /* Access via pointer with zero offset for char */
            char* cp = &ptr->d;
            sum += *cp;         /* [reg + 0] */
            
            ptr++;  /* Pointer increment by struct size */
        }
    }
    
    /* ===== PATTERN 6: Multiple base registers with same array ===== */
    {
        int* p1 = arr_i;
        int* p2 = arr_i + 100;
        float* f1 = arr_f;
        float* f2 = arr_f + 100;
        
        /* All these should generate [reg + 0] addressing */
        for (i = 0; i < 50; i++) {
            sum += *p1;     /* Direct dereference - zero offset */
            sum += *p2;
            fsum += *f1;
            fsum += *f2;
            
            /* Pointer arithmetic that might fold to auto-inc */
            p1 += 1;  /* Could become post-increment in previous access */
            p2 += 1;
            f1 += 1;
            f2 += 1;
        }
    }
    
    /* ===== PATTERN 7: Call helper functions ===== */
    sum += process_subset(arr_i, 100);
    fsum += process_structs(arr_struct, 50);
    
    /* ===== PATTERN 8: Complex loop with multiple induction variables ===== */
    {
        int idx = 0;
        int* base = arr_i;
        
        /* Mix pointer and index arithmetic */
        for (i = 0; i < 100; i += 2) {
            int* ptr = base + idx;  /* Base + index computation */
            sum += *ptr;            /* [reg + 0] */
            sum += *(ptr + 1);      /* [reg + 4] (assuming 4-byte int) */
            idx += 2;
            
            /* Array access with index modification */
            sum += arr_s[idx++];    /* Different type, different size */
            sum += arr_c[--idx];    /* Reset index */
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: int=%d, float=%.2f, double=%.2f\n", 
           sum, fsum, dsum);
    
    return 0;
}
