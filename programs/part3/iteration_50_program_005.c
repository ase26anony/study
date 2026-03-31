/* auto-inc-dec-trigger.c
 * Generated to specifically trigger the uncovered lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec -o auto-inc-dec-trigger auto-inc-dec-trigger.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#pragma GCC optimize("O3")

/* Heterogeneous structure with mixed alignment requirements */
struct S {
    char c;
    short s;
    int a;
    float b;
    double d;
    long l;
};

/* __attribute__((noinline)) helper to create another context for find_inc */
__attribute__((noinline))
static double process_subset(int* restrict arr_i, float* restrict arr_f, struct S* restrict arr_s, int n) {
    double sum = 0.0;
    int* p = arr_i;
    float* q = arr_f;
    struct S* r = arr_s;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* [reg + 0] addressing */
        sum += *q;      /* Another [reg + 0] */
        sum += r->a;    /* Structure field: base + 0 offset for first field */
        p++;
        q++;
        r++;
    }
    
    /* Insert compiler barrier via function call */
    sum += sin(sum);
    
    /* Pattern 2: Mixed pointer and index arithmetic */
    r = arr_s;
    for (int i = 0; i < n; i++) {
        /* Force [reg + 0] addressing for different members */
        sum += r->c;    /* char access */
        sum += r->s;    /* short access */
        sum += r->d;    /* double access */
        r = &arr_s[i];  /* Reset pointer with index */
    }
    
    return sum;
}

int main(void) {
    srand(time(NULL));
    
    /* Declare arrays of different types and alignments */
    int arr_i[100];
    float arr_f[100];
    double arr_d[100];
    short arr_s[100];
    struct S arr_struct[50];
    
    /* Initialize with random values */
    for (int i = 0; i < 100; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
        arr_s[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 50; i++) {
        arr_struct[i].c = (char)(rand() % 256);
        arr_struct[i].s = (short)(rand() % 1000);
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].d = (double)(rand() % 1000) / 10.0;
        arr_struct[i].l = rand() % 1000;
    }
    
    double total_sum = 0.0;
    
    /* ===== PATTERN 1: Post-increment in for loop ===== */
    {
        register int* p = arr_i;  /* register hint */
        int sum = 0;
        
        #pragma GCC unroll 4
        for (int i = 0; i < 100; i++) {
            sum += *p++;  /* Post-increment access */
        }
        total_sum += sum;
        
        /* Compiler barrier */
        total_sum += cos(total_sum);
    }
    
    /* ===== PATTERN 2: Post-decrement in while loop ===== */
    {
        float* q = &arr_f[99];
        float sum = 0.0f;
        int count = 100;
        
        while (count-- > 0) {
            sum += *q--;  /* Post-decrement access */
        }
        total_sum += sum;
        
        /* Reset pointer with zero offset */
        q = arr_f;  /* [reg + 0] addressing */
        total_sum += *q;  /* Single access with zero offset */
    }
    
    /* ===== PATTERN 3: Nested loops with pointer reset ===== */
    {
        struct S* ptr = arr_struct;
        double sum = 0.0;
        
        /* Outer loop resets pointer */
        for (int outer = 0; outer < 3; outer++) {
            ptr = arr_struct;  /* Reset to base */
            
            /* Inner loop uses pointer increment */
            for (int inner = 0; inner < 50; inner++) {
                /* Structure field accesses - decompose to base + constant offset */
                sum += ptr->a;    /* First field: offset 0 */
                sum += ptr->b;    /* Second field: non-zero offset */
                sum += ptr->d;    /* Third field: larger offset */
                ptr++;            /* Pointer increment */
            }
            
            /* Compiler barrier between outer iterations */
            asm volatile("" ::: "memory");
        }
        total_sum += sum;
    }
    
    /* ===== PATTERN 4: Mixed pointer and index arithmetic ===== */
    {
        int* base = arr_i;
        int sum = 0;
        
        /* Mix pointer and index in same loop */
        for (int i = 0; i < 100; i++) {
            /* Force [reg + 0] then update */
            int* tmp = base + i;  /* Create [reg + const] addressing */
            sum += *tmp;          /* Use with zero offset from tmp */
            
            /* Also use direct array indexing with increment */
            sum += arr_i[i];      /* Base + index addressing */
        }
        total_sum += sum;
        
        /* Function call obscures aliasing */
        total_sum += sqrt(fabs(total_sum));
    }
    
    /* ===== PATTERN 5: Compound expressions in loop conditions ===== */
    {
        int idx = 0;
        int sum = 0;
        
        /* arr[index++] in condition */
        while (idx < 100) {
            sum += arr_i[idx++];  /* Compound expression with post-increment */
        }
        total_sum += sum;
        
        idx = 100;
        sum = 0;
        
        /* arr[--index] in update */
        for (int i = 0; i < 100; i++) {
            sum += arr_s[--idx];  /* Pre-decrement in array access */
        }
        total_sum += sum;
    }
    
    /* ===== PATTERN 6: Multiple pointer aliases with zero offset ===== */
    {
        int* p1 = arr_i;
        int* p2 = arr_i + 50;
        float* f1 = arr_f;
        double* d1 = arr_d;
        
        /* All use [reg + 0] addressing */
        total_sum += *p1;
        total_sum += *p2;
        total_sum += *f1;
        total_sum += *d1;
        
        /* Structure with first field access (offset 0) */
        total_sum += arr_struct[0].a;
    }
    
    /* ===== PATTERN 7: Call helper function for another context ===== */
    total_sum += process_subset(arr_i, arr_f, arr_struct, 25);
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %f\n", total_sum);
    
    return 0;
}
