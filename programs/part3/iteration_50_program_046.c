/* auto-inc-dec-test.c */
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
    char pad[7];
};

/* __attribute__((noinline)) helper to create another find_inc context */
__attribute__((noinline)) 
static double process_subset(int* RESTRICT arr, int n) {
    double sum = 0.0;
    int* p = arr;
    int* end = arr + n;
    
    /* Pattern: [reg + 0] addressing with post-increment */
    while (p < end) {
        sum += *p++;  /* Should trigger find_inc with reg1_val = 0 */
    }
    
    /* Insert function call to force re-evaluation */
    sum += sin(sum);
    
    /* Another pattern with pointer arithmetic */
    for (p = arr; p < end; p += 2) {
        sum += *(p + 0);  /* Explicit [reg + 0] addressing */
    }
    
    return sum;
}

/* Function with aggressive optimization */
__attribute__((optimize("O3")))
static void process_with_auto_inc(int* RESTRICT ptr, int count) {
    register int* rp asm ("r12") = ptr;  /* register hint */
    int sum = 0;
    
    /* Loop with post-increment in access */
    for (int i = 0; i < count; i++) {
        sum += *rp++;
    }
    
    /* Use result to prevent elimination */
    volatile int dummy = sum;
    (void)dummy;
}

int main(void) {
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    struct Heterogeneous arr_s[100];
    
    /* Initialize with varying values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)rand() / RAND_MAX;
        arr_d[i] = (double)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < 100; i++) {
        arr_s[i].c = rand() % 256;
        arr_s[i].s = rand() % 1000;
        arr_s[i].i = rand();
        arr_s[i].l = rand() * 1000L;
        arr_s[i].f = (float)rand() / RAND_MAX;
        arr_s[i].d = (double)rand() / RAND_MAX;
    }
    
    long long total_sum = 0;
    
    /* PATTERN 1: Simple post-increment with int pointer */
    {
        int* p = arr_i;
        int* end = arr_i + 100;
        
        /* This should trigger find_inc with mem_insn.reg1_val = 0 */
        while (p < end) {
            total_sum += *p++;  /* Post-increment access */
        }
        
        /* Compiler barrier via math function */
        total_sum += (long long)sin(total_sum);
    }
    
    /* PATTERN 2: Post-decrement with float pointer */
    {
        float* q = arr_f + 99;
        float* start = arr_f;
        
        /* Post-decrement pattern */
        while (q >= start) {
            total_sum += (long long)(*q-- * 1000);
        }
        
        /* Reset and use array indexing with increment */
        for (int idx = 0; idx < 100; idx++) {
            total_sum += (long long)arr_f[idx++];  /* Compound increment */
        }
    }
    
    /* PATTERN 3: Nested loops with struct access */
    {
        struct Heterogeneous* ptr = arr_s;
        
        /* Outer loop resets pointer */
        for (int outer = 0; outer < 3; outer++) {
            struct Heterogeneous* inner_ptr = ptr;
            
            /* Inner loop uses pointer increment */
            for (int i = 0; i < 20; i++) {
                /* Multiple [reg + 0] accesses (first field of struct) */
                total_sum += inner_ptr->c;  /* Base + 0 offset */
                total_sum += inner_ptr->s;  /* Base + 2 offset (after char) */
                inner_ptr++;  /* Pointer increment */
            }
            
            /* Function call between pointer operations */
            total_sum += (long long)cos(ptr->f);
        }
    }
    
    /* PATTERN 4: Mixed pointer and index arithmetic */
    {
        double* base = arr_d;
        
        /* Mix pointer and index in same loop */
        for (int i = 0; i < 100; i++) {
            /* [reg + 0] addressing where reg = base + i */
            total_sum += (long long)*(base + i);  /* Pointer arithmetic */
        }
        
        /* Another variant with explicit zero offset */
        double* r = arr_d;
        for (int j = 0; j < 100; j++) {
            total_sum += (long long)*(r + 0);  /* Explicit zero offset */
            r++;  /* Separate increment */
        }
    }
    
    /* PATTERN 5: Multiple pointer aliases with zero offset */
    {
        int* p1 = arr_i + 50;
        int* p2 = arr_i + 75;
        int* p3 = arr_i + 90;
        
        /* Different pointers, all with [reg + 0] addressing */
        for (int i = 0; i < 10; i++) {
            total_sum += p1[0];  /* [p1 + 0] */
            total_sum += p2[0];  /* [p2 + 0] */
            total_sum += p3[0];  /* [p3 + 0] */
            
            p1++;
            p2++;
            p3++;
        }
    }
    
    /* PATTERN 6: Loop with inline assembly barrier */
    {
        short* sp = (short*)arr_i;  /* Misaligned access for testing */
        
        #pragma GCC unroll 4
        for (int i = 0; i < 50; i++) {
            total_sum += *sp++;
            
            /* Inline assembly that clobbers memory */
            asm volatile("" ::: "memory");
        }
    }
    
    /* Call helper function for additional find_inc context */
    double subset_sum = process_subset(arr_i + 150, 25);
    total_sum += (long long)subset_sum;
    
    /* Call optimized function */
    process_with_auto_inc(arr_i, 50);
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %lld\n", total_sum);
    
    return 0;
}
