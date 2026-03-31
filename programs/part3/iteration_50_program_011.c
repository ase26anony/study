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
static int process_with_pointer(int *arr, int n) {
    register int *p = arr;  /* register hint for allocation */
    int sum = 0;
    
    /* Pattern 1: Simple post-increment in loop */
    for (int i = 0; i < n; i++) {
        sum += *p++;  /* Should trigger [reg + 0] with post-inc */
    }
    
    /* Insert function call to force re-evaluation */
    if (sum > 0) {
        sin((double)sum);  /* Compiler barrier */
    }
    
    return sum;
}

/* Another noinline helper with different pattern */
__attribute__((noinline))
static float process_floats(float *arr, int n) {
    float *q = arr + n - 1;  /* Start from end */
    float prod = 1.0f;
    
    /* Pattern 2: Post-decrement in while loop */
    while (q >= arr) {
        prod *= *q--;  /* Post-decrement pattern */
    }
    
    /* Mixed pointer/index arithmetic */
    float *r = arr;
    for (int i = 0; i < n; i++) {
        prod += *(r + i);  /* [reg + 0] with index */
    }
    
    return prod;
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
    
    int total_sum = 0;
    float total_prod = 1.0f;
    double total_dsum = 0.0;
    
    /* PATTERN 1: Multiple pointer variables with zero offset */
    {
        int *p1 = arr_i;
        int *p2 = arr_i + 100;
        float *f1 = arr_f;
        double *d1 = arr_d;
        
        /* Access with [reg + 0] pattern */
        total_sum += *p1;  /* Direct dereference */
        total_sum += *p2;
        total_prod *= *f1;
        total_dsum += *d1;
        
        /* Function call between operations */
        sin(total_dsum);
    }
    
    /* PATTERN 2: Nested loops with pointer reset */
    #pragma GCC unroll 4
    for (int outer = 0; outer < 4; outer++) {
        int *ptr = arr_i + (outer * 50);
        
        /* Inner loop with post-increment */
        for (int inner = 0; inner < 50; inner++) {
            total_sum += *ptr++;
        }
        
        /* Compiler barrier */
        cos((double)outer);
    }
    
    /* PATTERN 3: Struct array traversal with field access */
    {
        struct S *sp = arr_struct;
        struct S * __restrict sp2 = arr_struct + 50;  /* restrict for alias analysis */
        
        /* Access first field (offset 0) */
        for (int i = 0; i < 50; i++) {
            total_sum += sp->a;  /* Base + 0 offset */
            total_sum += sp2->a;
            sp++;
            sp2++;
        }
        
        /* Reset and access other fields */
        sp = arr_struct;
        for (int i = 0; i < 100; i++) {
            total_prod *= sp->b;  /* Base + non-zero offset */
            total_dsum += sp->c;
            sp++;
        }
    }
    
    /* PATTERN 4: Mixed pointer/index arithmetic */
    {
        char *cp = arr_c;
        short *sp = arr_s;
        
        /* Compound expressions with index modification */
        for (int idx = 0; idx < 200; idx++) {
            total_sum += arr_c[idx++];  /* arr[index++] pattern */
            total_sum += arr_s[--idx];  /* arr[--index] pattern */
            
            /* Pointer arithmetic with zero offset */
            total_sum += *(cp + 0);  /* Explicit [reg + 0] */
            total_sum += *(sp + 0);
            
            cp++;
            sp++;
        }
    }
    
    /* PATTERN 5: While loop with post-decrement */
    {
        float *q = arr_f + 199;
        while (q >= arr_f) {
            total_prod *= *q--;  /* Post-decrement */
            
            /* Inline assembly to clobber memory */
            __asm__ volatile ("" ::: "memory");
        }
    }
    
    /* PATTERN 6: Call helper functions */
    total_sum += process_with_pointer(arr_i, 100);
    total_prod *= process_floats(arr_f, 100);
    
    /* PATTERN 7: Complex expression with multiple bases */
    {
        int * __restrict rp1 = arr_i;
        int * __restrict rp2 = arr_i + 50;
        int * __restrict rp3 = arr_i + 100;
        
        for (int i = 0; i < 50; i++) {
            /* Multiple [reg + 0] accesses in same statement */
            total_sum += *rp1 + *rp2 + *rp3;
            rp1++; rp2++; rp3++;
            
            /* Function call between iterations */
            if (i % 10 == 0) {
                sqrt(fabs(total_prod));
            }
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: int=%d, float=%f, double=%lf\n", 
           total_sum, total_prod, total_dsum);
    
    return 0;
}
