#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Compiler hints to influence optimization */
#pragma GCC optimize("O3")
#define RESTRICT __restrict

/* Mixed data types for alignment testing */
struct Heterogeneous {
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
};

/* Helper function marked noinline to create separate context */
__attribute__((noinline)) 
static int process_subset(int* RESTRICT arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Pattern 1: Simple post-increment in while loop */
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

/* Another noinline helper with different access pattern */
__attribute__((noinline))
static float process_floats(float* RESTRICT farr, int n) {
    float sum = 0.0f;
    float* q = farr;
    
    /* Mix pointer and index arithmetic */
    for (int i = 0; i < n; i++) {
        sum += *(q + i);  /* [reg + 0] pattern when q is base */
    }
    
    /* Reset pointer and use pure pointer arithmetic */
    q = farr;
    for (int j = 0; j < n; j++) {
        sum += *q++;
    }
    
    return sum;
}

int main(void) {
    /* Arrays of different types and sizes */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    struct Heterogeneous arr_s[100];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 100; i++) {
        arr_s[i].c = (char)(rand() % 256);
        arr_s[i].s = (short)(rand() % 1000);
        arr_s[i].i = rand() % 1000;
        arr_s[i].l = rand() % 1000;
        arr_s[i].f = (float)(rand() % 1000) / 10.0f;
        arr_s[i].d = (double)(rand() % 1000) / 10.0;
    }
    
    int total_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* ===== PATTERN 1: Simple post-increment with int pointer ===== */
    {
        register int* p = arr_i;  /* register hint */
        int* end = arr_i + 100;
        
        /* Loop with post-increment */
        #pragma GCC unroll 4
        for (; p < end; p++) {
            total_sum += *p;
        }
        
        /* Function call as compiler barrier */
        float barrier = sinf((float)total_sum);
        (void)barrier;
    }
    
    /* ===== PATTERN 2: Post-decrement with float pointer ===== */
    {
        float* q = arr_f + 100;  /* Start from middle */
        float* start = arr_f;
        
        /* While loop with post-decrement */
        while (q > start) {
            float_sum += *q--;
        }
        
        /* Reset and use different pattern */
        q = arr_f;
        for (int i = 0; i < 100; i++) {
            float_sum += q[i];  /* Array indexing */
        }
        
        /* Another compiler barrier */
        double barrier2 = cos((double)float_sum);
        (void)barrier2;
    }
    
    /* ===== PATTERN 3: Nested loops with struct access ===== */
    {
        struct Heterogeneous* ptr = arr_s;
        
        /* Outer loop resets pointer */
        for (int outer = 0; outer < 2; outer++) {
            ptr = arr_s + (outer * 50);
            
            /* Inner loop uses pointer increment */
            for (int inner = 0; inner < 50; inner++) {
                /* Multiple field accesses from same base */
                total_sum += ptr->i;      /* First field, offset 0 */
                float_sum += ptr->f;
                double_sum += ptr->d;
                ptr++;
            }
            
            /* Inline assembly to clobber memory */
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* ===== PATTERN 4: Mixed pointer/index arithmetic ===== */
    {
        double* r = arr_d;
        int idx = 0;
        
        /* Mix pointer and index in same loop */
        for (idx = 0; idx < 100; idx++) {
            /* This should create [reg + 0] when optimized */
            double_sum += *(r + idx);
        }
        
        /* Reset and use pure pointer arithmetic */
        r = arr_d + 100;
        for (int j = 0; j < 100; j++) {
            double_sum += *(--r);  /* Pre-decrement */
        }
    }
    
    /* ===== PATTERN 5: Compound expressions in loop conditions ===== */
    {
        int index = 0;
        int* base = arr_i + 150;
        
        /* arr[index++] in condition */
        while (index < 50) {
            total_sum += base[index++];
        }
        
        /* arr[--index] in update */
        index = 50;
        int* alt_base = arr_i;
        for (int k = 0; k < 50; k++) {
            total_sum += alt_base[--index];
        }
        
        /* Compiler barrier with math function */
        float barrier3 = sqrtf(fabsf((float)total_sum));
        (void)barrier3;
    }
    
    /* ===== PATTERN 6: Multiple pointer aliases to same array ===== */
    {
        int* p1 = arr_i;
        int* p2 = arr_i;
        int* p3 = arr_i;
        
        /* Different pointers, same base, zero offset */
        for (int i = 0; i < 50; i++) {
            total_sum += *p1;  /* p1 with offset 0 */
            p1++;
            
            total_sum += p2[i];  /* p2[i] -> [p2 + i*4] */
            
            total_sum += *(p3 + i);  /* *(p3 + i) */
        }
    }
    
    /* ===== PATTERN 7: Call helper functions ===== */
    total_sum += process_subset(arr_i + 180, 20);
    float_sum += process_floats(arr_f + 180, 20);
    
    /* ===== PATTERN 8: Complex loop with multiple updates ===== */
    {
        char* cptr = (char*)arr_i;
        short* sptr = (short*)arr_i;
        
        /* Different sized accesses from same base */
        for (int i = 0; i < 100; i++) {
            total_sum += (int)cptr[i];
            total_sum += (int)sptr[i];
            
            /* Pointer arithmetic with different types */
            cptr += sizeof(int);
            sptr += sizeof(int) / sizeof(short);
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: int=%d, float=%.2f, double=%.2f\n", 
           total_sum, float_sum, double_sum);
    
    return 0;
}
