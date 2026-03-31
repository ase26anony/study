/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Heterogeneous structure with different alignments */
struct S {
    char c;
    short s;
    int a;
    float b;
    double d;
    long long ll;
} __attribute__((packed));

/* Helper function marked noinline to create separate optimization context */
__attribute__((noinline, optimize("O3")))
static double process_subset(int* base, float* fbase, struct S* sbase, int n) {
    register int* p __asm__("r12") = base;
    register float* q = fbase;
    double sum = 0.0;
    int i;
    
    /* Pattern 1: Pointer post-increment with zero offset */
    for (i = 0; i < n; i++) {
        sum += *p;          /* [reg + 0] addressing */
        p++;                /* Post-increment */
        
        /* Force re-evaluation with function call */
        if (i % 8 == 0) {
            sum += sin(*q);
            q++;
        }
    }
    
    /* Pattern 2: Structure access with mixed offsets */
    struct S* sp = sbase;
    for (i = 0; i < n/2; i++) {
        sum += sp->a;       /* Base + constant offset (may be 0 for first field) */
        sum += sp->b;
        sp++;
    }
    
    return sum;
}

/* Another helper with restrict to help alias analysis */
__attribute__((noinline))
static void process_with_restrict(int* __restrict p1, int* __restrict p2, int n) {
    int i;
    /* Mix pointer and index arithmetic */
    for (i = 0; i < n; i++) {
        *p1++ = *p2++ + i;  /* Both pointers increment */
    }
}

int main(void) {
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    short arr_s[200];
    struct S arr_struct[100];
    
    int i, j;
    double total_sum = 0.0;
    
    /* Initialize with random values */
    srand(42);
    for (i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
        arr_s[i] = (short)(rand() % 1000);
    }
    
    for (i = 0; i < 100; i++) {
        arr_struct[i].c = (char)(rand() % 256);
        arr_struct[i].s = (short)(rand() % 1000);
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].d = (double)(rand() % 1000) / 10.0;
        arr_struct[i].ll = (long long)rand() * rand();
    }
    
    /* PATTERN 1: Simple post-increment with int pointer */
    {
        int* p = arr_i;
        int sum = 0;
        
        #pragma GCC unroll 4
        for (i = 0; i < 100; i++) {
            sum += *p++;    /* Should trigger find_inc for [reg+0] */
        }
        
        /* Function call between pointer operations */
        total_sum += sin(sum);
        
        /* Continue with same pointer */
        for (; i < 150; i++) {
            sum += *p++;
        }
        total_sum += sum;
    }
    
    /* PATTERN 2: Post-decrement with float pointer */
    {
        float* q = &arr_f[149];
        float fsum = 0.0f;
        
        while (q >= &arr_f[50]) {
            fsum += *q--;   /* Post-decrement */
            
            /* Insert math function to force addressing re-evaluation */
            if ((int)(q - arr_f) % 16 == 0) {
                fsum += cos(fsum);
            }
        }
        total_sum += fsum;
    }
    
    /* PATTERN 3: Nested loops with pointer reset */
    {
        int* base_ptr = arr_i;
        
        for (j = 0; j < 4; j++) {
            int* p = base_ptr + j * 25;  /* Different base each iteration */
            int local_sum = 0;
            
            for (i = 0; i < 25; i++) {
                local_sum += *(p + i);  /* Pointer + index arithmetic */
            }
            
            /* Mix pointer and array indexing */
            for (i = 0; i < 25; i++) {
                local_sum += p[i];      /* Array notation */
                p++;                    /* Also increment pointer */
            }
            
            total_sum += local_sum;
        }
    }
    
    /* PATTERN 4: Structure array traversal */
    {
        struct S* sp = arr_struct;
        double struct_sum = 0.0;
        
        /* Access first field (offset 0) */
        for (i = 0; i < 50; i++) {
            struct_sum += sp->a;    /* Base + 0 offset for first int field */
            sp++;
        }
        
        /* Reset and access other fields */
        sp = arr_struct;
        for (i = 0; i < 50; i++) {
            struct_sum += sp->b;    /* Base + non-zero offset */
            struct_sum += sp->d;
            sp++;
        }
        
        total_sum += struct_sum;
    }
    
    /* PATTERN 5: Mixed data types with alignment considerations */
    {
        char* cp = (char*)arr_i;
        short* sp = arr_s;
        double* dp = arr_d;
        
        /* Process misaligned access */
        for (i = 0; i < 100; i++) {
            total_sum += cp[i * 4];     /* Access ints as chars */
            total_sum += sp[i];         /* 2-byte aligned */
            total_sum += dp[i];         /* 8-byte aligned */
        }
    }
    
    /* PATTERN 6: Compound expressions with index modification */
    {
        int idx = 0;
        int sum1 = 0, sum2 = 0;
        
        /* Pre-decrement in array access */
        for (i = 99; i >= 0; i--) {
            sum1 += arr_i[--idx + 100];  /* Modify index in expression */
        }
        
        /* Post-increment in array access */
        idx = 0;
        for (i = 0; i < 100; i++) {
            sum2 += arr_s[idx++];        /* Post-increment */
        }
        
        total_sum += sum1 + sum2;
    }
    
    /* Call helper functions for additional contexts */
    total_sum += process_subset(arr_i + 10, arr_f + 20, arr_struct + 5, 30);
    
    /* Use restrict to help alias analysis */
    process_with_restrict(arr_i + 60, arr_i + 80, 40);
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %f\n", total_sum);
    
    /* Use inline assembly as compiler barrier */
    __asm__ volatile ("" : : : "memory");
    
    return 0;
}
