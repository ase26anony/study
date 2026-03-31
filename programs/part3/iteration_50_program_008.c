/* auto-inc-dec-trigger.c
 * Generated to specifically target uncovered lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec -o auto_inc_dec_test auto-inc-dec-trigger.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Mixed data types for alignment testing */
struct Heterogeneous {
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
};

/* Restrict-qualified pointer helper */
__attribute__((noinline))
static double process_restricted(const int* __restrict rptr, int n) {
    double sum = 0.0;
    const int* p = rptr;
    while (n-- > 0) {
        sum += *p++;  /* Post-increment with zero offset potential */
    }
    return sum;
}

/* Function with optimize attribute to force specific optimization context */
__attribute__((optimize("O3")))
static void nested_pointer_ops(int* base, float* fbase, int count) {
    register int* p = base;      /* register hint for RTL decisions */
    register float* q = fbase;
    
    /* Outer loop resets pointer, inner uses auto-inc */
    for (int outer = 0; outer < 3; outer++) {
        p = base + outer * 10;   /* Different base each iteration */
        q = fbase + outer * 10;
        
        /* Inner loop with post-increment and function call barrier */
        for (int i = 0; i < 10; i++) {
            *p++ += (int)(*q++ * 100.0f);
            
            /* Function call that clobbers memory visibility */
            if ((i & 3) == 0) {
                volatile double barrier = sin((double)*p);
                (void)barrier;
            }
        }
    }
}

int main(void) {
    /* 1. Arrays of different types and alignments */
    int arr_i[100];
    float arr_f[100];
    double arr_d[100];
    struct Heterogeneous arr_s[50];
    
    /* 2. Initialize with varying values */
    srand(42);
    for (int i = 0; i < 100; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 5.0;
    }
    
    for (int i = 0; i < 50; i++) {
        arr_s[i].c = (char)(rand() % 256);
        arr_s[i].s = (short)(rand() % 1000);
        arr_s[i].i = rand() % 1000;
        arr_s[i].l = rand() % 1000;
        arr_s[i].f = (float)(rand() % 1000) / 10.0f;
        arr_s[i].d = (double)(rand() % 1000) / 5.0;
    }
    
    double total_sum = 0.0;
    
    /* 3a. Pattern A: for loop with *p++ */
    {
        int sum = 0;
        int* p = arr_i;
        #pragma GCC unroll 4
        for (int i = 0; i < 100; i++) {
            sum += *p++;  /* Direct post-increment */
        }
        total_sum += sum;
        
        /* Compiler barrier via math function */
        total_sum += sin((double)sum);
    }
    
    /* 3b. Pattern B: while loop with *q-- and reset */
    {
        float sum_f = 0.0f;
        float* q = &arr_f[99];  /* Start from end */
        int counter = 100;
        
        while (counter-- > 0) {
            sum_f += *q--;      /* Post-decrement */
            
            /* Reset pointer mid-way to create multiple base registers */
            if (counter == 50) {
                q = &arr_f[49];  /* New base with zero offset potential */
            }
        }
        total_sum += sum_f;
    }
    
    /* 3c. Pattern C: Nested struct access with ptr++ */
    {
        long struct_sum = 0;
        struct Heterogeneous* ptr = arr_s;
        
        for (int outer = 0; outer < 2; outer++) {
            ptr = arr_s + outer * 25;  /* Different base each outer iteration */
            
            /* Access first field (offset 0) then others */
            for (int i = 0; i < 25; i++) {
                struct_sum += ptr->i;    /* Base + 0 offset for first int field */
                total_sum += ptr->f;     /* Different offset */
                ptr++;                   /* Post-increment */
            }
            
            /* Function call between pointer operations */
            total_sum += cos((double)struct_sum);
        }
    }
    
    /* 3d. Pattern D: Mixed integer index and pointer arithmetic */
    {
        double* base = arr_d;
        double sum_d = 0.0;
        
        /* Mix pointer and index in same expression */
        for (int idx = 0; idx < 100; idx++) {
            /* *(base + idx) pattern where idx changes */
            sum_d += *(base + idx);
            
            /* Also use direct indexing with increment */
            if (idx % 10 == 0) {
                sum_d += arr_d[idx++];  /* Compound expression with side effect */
            }
        }
        total_sum += sum_d;
    }
    
    /* 4. Call nested function with register pointers */
    nested_pointer_ops(arr_i, arr_f, 30);
    
    /* 5. Use restrict-qualified pointer function */
    total_sum += process_restricted(arr_i, 50);
    
    /* 6. Additional pattern: array indexing with pre/post inc/dec */
    {
        int idx = 0;
        int sum = 0;
        
        /* Compound expressions in loop conditions */
        while (idx < 100) {
            sum += arr_i[idx++];      /* Post-increment in array index */
            
            if (idx < 50) {
                sum -= arr_i[--idx];  /* Pre-decrement in array index */
                idx += 2;
            }
        }
        total_sum += sum;
    }
    
    /* 7. Mixed-size type traversal */
    {
        char* cptr = (char*)arr_i;
        int* iptr = arr_i;
        
        /* Different pointer types accessing same memory */
        for (int i = 0; i < 10; i++) {
            *cptr++ = (char)(*iptr++ & 0xFF);
            
            /* Inline asm as memory clobber */
            __asm__ volatile ("" : : "r"(cptr), "r"(iptr) : "memory");
        }
    }
    
    /* Final checksum to prevent elimination */
    printf("Checksum: %f\n", total_sum);
    
    return 0;
}
