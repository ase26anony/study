/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")
#pragma GCC push_options

/* Heterogeneous structure with different alignments */
struct S {
    char c;
    short s;
    int a;
    float b;
    double d;
    long long ll;
} __attribute__((packed));

/* Helper function marked noinline to create separate context */
__attribute__((noinline, optimize("O3")))
static double process_subset(int* base, float* fbase, struct S* sbase, int n) {
    double sum = 0.0;
    int* restrict p = base;
    float* restrict q = fbase;
    struct S* restrict r = sbase;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *p;  /* [reg + 0] addressing */
        sum += *q;  /* Another [reg + 0] */
        sum += r->a; /* Structure field - base + 0 offset for first int field */
        p++;
        q++;
        r++;
    }
    
    /* Compiler barrier via math function */
    sum = sin(sum);
    
    /* Pattern 2: Post-decrement with while loop */
    p = &base[n-1];
    q = &fbase[n-1];
    r = &sbase[n-1];
    
    while (p >= base) {
        sum += *p--;  /* Post-decrement */
        sum += *q--;  /* Another post-decrement */
        sum += r->b;  /* Access float field */
        r--;
    }
    
    return sum;
}

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O3"), noinline))
static void mixed_access_patterns(int* arr_i, float* arr_f, struct S* arr_s, 
                                  int size_i, int size_f, int size_s) {
    double total = 0.0;
    register int* p asm ("r12");  /* Hint for register allocation */
    register float* q asm ("r13");
    register struct S* r asm ("r14");
    
    /* Pattern 3: Loop with pointer arithmetic and index modification */
    for (int i = 0; i < size_i; i++) {
        /* Mixed: array indexing with post-increment */
        total += arr_i[i++];  /* Compound expression */
        if (i < size_i) {
            total += arr_i[--i];  /* Pre-decrement in index */
        }
        
        /* Pointer arithmetic with constant 0 offset */
        p = arr_i + i;
        total += *p;  /* [reg + 0] */
        
        /* Compiler barrier */
        if (i % 10 == 0) {
            total = cos(total);
        }
    }
    
    /* Pattern 4: Nested loops with pointer reset */
    for (int outer = 0; outer < 3; outer++) {
        q = arr_f;  /* Reset pointer each outer iteration */
        #pragma GCC unroll 4
        for (int inner = 0; inner < size_f; inner++) {
            total += *q++;  /* Post-increment in inner loop */
            
            /* Access with different offset calculations */
            if (inner % 2 == 0) {
                total += *(q + 0);  /* Explicit [reg + 0] */
            }
        }
        
        /* Function call between pointer operations */
        total = sqrt(fabs(total));
    }
    
    /* Pattern 5: Structure array traversal with field access */
    r = arr_s;
    for (int i = 0; i < size_s; i++) {
        /* Different field accesses - all decompose to base + constant offset */
        total += r->a;    /* offset 0 (after packed char/short) */
        total += r->b;    /* offset 4 */
        total += r->d;    /* offset 8 */
        total += r->ll;   /* offset 16 */
        
        r++;  /* Pointer increment by structure size */
        
        /* Inline assembly to clobber memory */
        asm volatile("" : : : "memory");
    }
    
    printf("Mixed patterns checksum: %f\n", total);
}

int main(void) {
    /* Declare arrays of different types and alignments */
    int arr_i[100] __attribute__((aligned(16)));
    float arr_f[100] __attribute__((aligned(8)));
    double arr_d[50];
    struct S arr_s[50];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 100; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        if (i < 50) {
            arr_d[i] = (double)(rand() % 1000) / 100.0;
            arr_s[i].c = (char)(rand() % 256);
            arr_s[i].s = (short)(rand() % 1000);
            arr_s[i].a = rand() % 1000;
            arr_s[i].b = (float)(rand() % 1000) / 10.0f;
            arr_s[i].d = (double)(rand() % 1000) / 100.0;
            arr_s[i].ll = (long long)rand() * rand();
        }
    }
    
    double checksum = 0.0;
    
    /* Pattern A: Simple for loop with post-increment */
    {
        int* p = arr_i;
        int* end = arr_i + 100;
        #pragma GCC unroll 8
        while (p < end) {
            checksum += *p++;  /* Classic post-increment */
        }
        
        /* Compiler barrier */
        checksum = sin(checksum);
    }
    
    /* Pattern B: While loop with post-decrement and reset */
    {
        float* q = arr_f + 99;  /* Start from end */
        while (q >= arr_f) {
            checksum += *q--;  /* Post-decrement */
            
            /* Access with zero offset through alias */
            float* alias = q + 1;
            checksum += *alias;  /* [reg + 0] */
        }
        
        /* Reset and traverse forward */
        q = arr_f;
        for (int i = 0; i < 100; i++) {
            checksum += *(q + i);  /* Pointer + index */
        }
    }
    
    /* Pattern C: Nested loops with induction variables */
    {
        struct S* r = arr_s;
        for (int outer = 0; outer < 2; outer++) {
            struct S* inner_ptr = r;
            for (int inner = 0; inner < 25; inner++) {
                /* Multiple field accesses from same base */
                checksum += inner_ptr->a;
                checksum += inner_ptr->b;
                inner_ptr++;  /* Pointer increment */
            }
            
            /* Switch to different part of array */
            r = &arr_s[25];
            
            /* Compiler barrier */
            checksum = cos(checksum);
        }
    }
    
    /* Pattern D: Mixed integer index and pointer arithmetic */
    {
        int* base = arr_i;
        for (int idx = 0; idx < 100; idx++) {
            /* Calculate pointer from base + idx */
            int* ptr = base + idx;
            checksum += *ptr;  /* [reg + 0] after calculation */
            
            /* Also access with array syntax */
            checksum += arr_i[idx];
            
            /* Modify index in complex way */
            idx += (int)(sin(idx) * 0.0);  /* Compiler can't predict */
        }
    }
    
    /* Call helper function for additional context */
    checksum += process_subset(arr_i, arr_f, arr_s, 25);
    
    /* Call function with mixed patterns */
    mixed_access_patterns(arr_i, arr_f, arr_s, 100, 100, 50);
    
    /* Final checksum to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    
    return 0;
}

#pragma GCC pop_options
