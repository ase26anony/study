/* Compile with: gcc -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec auto-inc-dec-test.c -lm */
/* Also try: gcc -O3 -funroll-loops -fno-omit-frame-pointer -fno-schedule-insns auto-inc-dec-test.c -lm */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 100
#define STRUCT_SIZE 50

#pragma GCC optimize("O3")

/* Heterogeneous structure with different alignments */
struct S {
    char c;
    int a;
    float b;
    double d;
    short s;
} __attribute__((packed));

/* __attribute__((noinline)) helper function to create another context */
__attribute__((noinline)) 
static double process_subset(int *arr, float *farr, struct S *sarr, int n) {
    register int *p asm ("r12") = arr;
    register float *q asm ("r13") = farr;
    double sum = 0.0;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *p;  /* [reg + 0] addressing */
        sum += *q;  /* Another [reg + 0] addressing */
        p++;
        q++;
    }
    
    /* Pattern 2: Structure field access with zero offset for first field */
    struct S *sp = sarr;
    for (int i = 0; i < n/2; i++) {
        sum += sp->c;  /* Base + 0 offset */
        sum += sp->a;  /* Base + offset (not zero) */
        sp++;
    }
    
    return sum;
}

int main(void) {
    /* Declare arrays of different types and alignments */
    int arr_i[SIZE] __attribute__((aligned(16)));
    float arr_f[SIZE] __attribute__((aligned(8)));
    double arr_d[SIZE];
    char arr_c[SIZE*2];
    short arr_s[SIZE];
    struct S arr_struct[STRUCT_SIZE];
    
    /* Initialize with random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
        arr_c[i] = rand() % 256;
        arr_s[i] = rand() % 65536;
    }
    
    for (int i = 0; i < STRUCT_SIZE; i++) {
        arr_struct[i].c = rand() % 256;
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].d = (double)(rand() % 1000) / 10.0;
        arr_struct[i].s = rand() % 65536;
    }
    
    double total_sum = 0.0;
    
    /* ===== PATTERN 1: Post-increment in for loop ===== */
    {
        int sum = 0;
        /* Use register hint and restrict to help alias analysis */
        register int * __restrict p = arr_i;
        
        #pragma GCC unroll 4
        for (int i = 0; i < SIZE; i++) {
            /* This should trigger find_inc with [reg + 0] addressing */
            sum += *p++;  /* Post-increment */
        }
        total_sum += sum;
        
        /* Compiler barrier via math function */
        total_sum += sin(sum * 0.01);
    }
    
    /* ===== PATTERN 2: Post-decrement in while loop ===== */
    {
        float sum_f = 0.0f;
        float *q = &arr_f[SIZE-1];
        int count = SIZE;
        
        while (count-- > 0) {
            /* Post-decrement pattern */
            sum_f += *q--;
        }
        total_sum += sum_f;
        
        /* Another compiler barrier */
        total_sum += cos(sum_f);
    }
    
    /* ===== PATTERN 3: Nested loops with pointer reset ===== */
    {
        double sum_d = 0.0;
        int outer_loops = 5;
        
        for (int outer = 0; outer < outer_loops; outer++) {
            double *r = arr_d;
            /* Inner loop with pointer stepping */
            for (int i = 0; i < SIZE/2; i++) {
                /* Mixed: pointer arithmetic with index modification */
                sum_d += *(r + i);  /* [reg + offset] addressing */
            }
            
            /* Reset pointer for next iteration */
            r = arr_d + SIZE/2;
            for (int i = 0; i < SIZE/2; i++) {
                sum_d += *r++;
            }
        }
        total_sum += sum_d;
    }
    
    /* ===== PATTERN 4: Structure array traversal ===== */
    {
        struct S *sp = arr_struct;
        double struct_sum = 0.0;
        
        /* Access different structure fields */
        for (int i = 0; i < STRUCT_SIZE; i++) {
            /* These decompose to base + constant_offset */
            struct_sum += sp->c;  /* offset 0 */
            struct_sum += sp->a;  /* offset 1 (after char) */
            struct_sum += sp->b;  /* offset 5 */
            struct_sum += sp->d;  /* offset 9 */
            struct_sum += sp->s;  /* offset 17 */
            sp++;
        }
        total_sum += struct_sum;
        
        /* Compiler barrier */
        total_sum += sqrt(fabs(struct_sum));
    }
    
    /* ===== PATTERN 5: Mixed pointer and index arithmetic ===== */
    {
        int idx_sum = 0;
        int *base = arr_i;
        
        /* Mix pointer and index in same loop */
        for (int i = 0; i < SIZE; i++) {
            /* Compound expression with index modification */
            idx_sum += arr_i[i];  /* Array indexing */
            idx_sum += *(base + i);  /* Pointer arithmetic */
            
            /* Also try: arr_i[i++] in same statement (commented for correctness) */
            /* But we can do: */
            if (i % 2 == 0) {
                idx_sum += arr_s[i/2];  /* Different type, different base */
            }
        }
        total_sum += idx_sum;
    }
    
    /* ===== PATTERN 6: Multiple aliases to same memory ===== */
    {
        int sum_multi = 0;
        int *p1 = arr_i;
        int *p2 = arr_i + 10;
        int *p3 = arr_i + 20;
        
        /* Multiple pointers with zero-offset accesses */
        for (int i = 0; i < 30; i++) {
            if (i < 10) sum_multi += *p1++;
            else if (i < 20) sum_multi += *p2++;
            else sum_multi += *p3++;
        }
        total_sum += sum_multi;
    }
    
    /* ===== PATTERN 7: Call helper function ===== */
    total_sum += process_subset(arr_i, arr_f, arr_struct, 20);
    
    /* ===== PATTERN 8: Complex loop with if conditions ===== */
    {
        char *cp = arr_c;
        int byte_sum = 0;
        int index = 0;
        
        /* Loop with index modification in access */
        while (index < SIZE*2) {
            /* Compound access with increment */
            byte_sum += arr_c[index++];
            
            /* Also access via pointer */
            if (index % 4 == 0) {
                byte_sum += *cp;
                cp += 4;
            }
            
            /* Compiler barrier inside loop */
            if (index % 16 == 0) {
                byte_sum += (int)sin(byte_sum * 0.001);
            }
        }
        total_sum += byte_sum;
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %f\n", total_sum);
    
    return 0;
}
