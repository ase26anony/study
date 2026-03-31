/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Heterogeneous struct for mixed data types */
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
static long process_subset(int* arr, int n) {
    register int* p = arr;
    long sum = 0;
    
    /* Pattern 1: Post-increment in loop */
    for (int i = 0; i < n; i++) {
        sum += *p++;
    }
    
    /* Pattern 2: Post-decrement with reset */
    p = &arr[n-1];
    while (p >= arr) {
        sum += *p--;
    }
    
    return sum;
}

/* Another noinline helper with different pattern */
__attribute__((noinline))
static double process_floats(float* __restrict farr, int n) {
    register float* q = farr;
    double sum = 0.0;
    
    /* Mixed pointer and index arithmetic */
    for (int i = 0; i < n; i++) {
        sum += *(q + i);  /* [reg + 0] addressing when i=0 */
        if (i % 2 == 0) {
            sum += *q;    /* Direct dereference */
        }
    }
    
    /* Compound expression in update */
    int idx = 0;
    while (idx < n) {
        sum += farr[idx++];  /* arr[index++] pattern */
    }
    
    return sum;
}

int main(void) {
    /* Declare arrays of different types and sizes */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    char arr_c[200];
    short arr_s[200];
    struct S arr_struct[100];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
        arr_c[i] = (char)(rand() % 256);
        arr_s[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 100; i++) {
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].c = (double)(rand() % 1000) / 10.0;
        arr_struct[i].d = (char)(rand() % 256);
        arr_struct[i].e = (short)(rand() % 1000);
        arr_struct[i].f = (long)(rand() % 1000);
    }
    
    long total_sum = 0;
    double float_sum = 0.0;
    double struct_sum = 0.0;
    
    /* ===== PATTERN 1: Simple post-increment ===== */
    {
        int* __restrict p = arr_i;
        int* __restrict q = arr_f;
        int* __restrict r = arr_d;
        
        #pragma GCC unroll 4
        for (int i = 0; i < 100; i++) {
            total_sum += *p++;  /* Post-increment */
            float_sum += *q++;  /* Multiple pointers */
            struct_sum += *r++; /* Different base registers */
        }
        
        /* Function call as compiler barrier */
        float_sum += sin(float_sum);
    }
    
    /* ===== PATTERN 2: Post-decrement in while loop ===== */
    {
        float* q = &arr_f[199];
        int count = 200;
        
        while (count-- > 0) {
            float_sum += *q--;  /* Post-decrement */
            
            /* Mix with array indexing */
            if (count % 10 == 0) {
                float_sum += arr_f[count];  /* arr[--index] equivalent */
            }
        }
        
        /* Another compiler barrier */
        struct_sum += cos(struct_sum);
    }
    
    /* ===== PATTERN 3: Nested loops with struct access ===== */
    {
        struct S* ptr = arr_struct;
        
        /* Outer loop resets pointer */
        for (int outer = 0; outer < 3; outer++) {
            ptr = arr_struct;  /* Reset pointer */
            
            /* Inner loop uses pointer increment */
            for (int inner = 0; inner < 50; inner++) {
                /* Structure field accesses - base + constant offset (often 0 for first field) */
                total_sum += ptr->a;    /* [reg + 0] for first field */
                float_sum += ptr->b;
                struct_sum += ptr->c;
                
                ptr++;  /* Pointer increment */
            }
            
            /* Inline assembly to clobber memory */
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* ===== PATTERN 4: Mixed pointer and index arithmetic ===== */
    {
        int* base = arr_i;
        register int idx = 0;
        
        /* Mix pointer and index in same loop */
        for (idx = 0; idx < 100; idx++) {
            /* ptr = &base + idx pattern */
            int* current = base + idx;
            total_sum += *current;  /* [reg + 0] when idx=0 */
            
            /* Also use direct indexing */
            total_sum += base[idx];  /* Another [reg + 0] opportunity */
        }
        
        /* Complex expression with side effects */
        idx = 0;
        while (idx < 100) {
            total_sum += *(base + idx++);  /* Compound expression */
            
            /* Function call between operations */
            if (idx % 25 == 0) {
                float_sum += sqrt(fabs(float_sum));
            }
        }
    }
    
    /* ===== PATTERN 5: Different alignments and sizes ===== */
    {
        char* cp = arr_c;
        short* sp = arr_s;
        int* ip = arr_i;
        double* dp = arr_d;
        
        /* Process different types with different strides */
        for (int i = 0; i < 50; i++) {
            /* Different alignments */
            total_sum += *cp++;
            total_sum += *sp++;
            total_sum += *ip++;
            struct_sum += *dp++;
            
            /* Reset some pointers to create multiple base registers */
            if (i % 10 == 0) {
                cp = arr_c + i;
                sp = arr_s + i;
                ip = arr_i + i;
                dp = arr_d + i;
            }
        }
    }
    
    /* ===== PATTERN 6: Call helper functions ===== */
    {
        /* These create new contexts for find_inc */
        total_sum += process_subset(arr_i, 100);
        float_sum += process_floats(arr_f, 100);
        
        /* More compiler barriers */
        __asm__ volatile ("" : : : "memory");
        float_sum = sin(float_sum) + cos(float_sum);
    }
    
    /* ===== PATTERN 7: Complex loop with multiple updates ===== */
    {
        int* p1 = arr_i;
        int* p2 = arr_i + 100;
        int* p3 = arr_i + 150;
        
        /* Multiple pointers updated in same loop */
        for (int i = 0; i < 50; i++) {
            total_sum += *p1++ + *p2++ + *p3++;
            
            /* Access with zero offset through different pointers */
            if (i == 0) {
                total_sum += *arr_i;      /* [reg + 0] */
                total_sum += *(arr_i+0);  /* Explicit zero offset */
            }
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: total_sum = %ld, float_sum = %f, struct_sum = %f\n",
           total_sum, float_sum, struct_sum);
    
    return (int)(total_sum + float_sum + struct_sum) % 256;
}
