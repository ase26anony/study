/* auto-inc-dec-trigger.c
 * 
 * This program is specifically designed to trigger the uncovered lines
 * 1352-1358 in auto-inc-dec.cc by creating patterns that force the
 * compiler to use auto-increment/decrement addressing modes.
 *
 * Compilation recommendations:
 *   gcc -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec auto-inc-dec-trigger.c -lm
 *   gcc -O3 -funroll-loops -fno-omit-frame-pointer -fno-schedule-insns auto-inc-dec-trigger.c -lm
 *   gcc -O1 -da -fdump-rtl-all -fno-expensive-optimizations auto-inc-dec-trigger.c -lm
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

/* Simple struct for basic pattern */
struct S {
    int a;
    float b;
    char pad[7]; /* Force non-power-of-two size */
};

/* __attribute__((noinline)) helper to create another context */
__attribute__((noinline))
static double process_subset(int* base, int size) {
    register int* p asm ("r12") = base; /* Hint for register allocation */
    double sum = 0.0;
    int i;
    
    /* Pattern 1: Simple post-increment with zero offset */
    for (i = 0; i < size; i++) {
        sum += *p;  /* Should become [reg + 0] */
        p++;        /* Post-increment */
    }
    
    /* Pattern 2: Mixed pointer/index with constant offset */
    int* q = base;
    for (i = 0; i < size/2; i++) {
        sum += *(q + 0);  /* Explicit zero offset */
        q = &q[1];        /* Another form of increment */
    }
    
    return sum;
}

/* Function with aggressive optimization attributes */
__attribute__((optimize("O3")))
static void process_with_restrict(int* __restrict p1, float* __restrict p2, 
                                  struct S* __restrict p3, int n) {
    int i;
    float temp = 0.0f;
    
    #pragma GCC unroll 4
    for (i = 0; i < n; i++) {
        /* Multiple zero-offset accesses with different base registers */
        int val1 = *p1;      /* [reg1 + 0] */
        float val2 = *p2;    /* [reg2 + 0] */
        int val3 = p3->a;    /* [reg3 + 0] for first field */
        
        temp += val1 * val2 + val3;
        
        /* Post-increment all pointers */
        p1++;
        p2++;
        p3++;
    }
    
    /* Use temp to prevent elimination */
    asm volatile("" : "+r"(temp));
}

int main(void) {
    /* Declare arrays of different types */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    struct S arr_s[100];
    struct Heterogeneous arr_h[50];
    
    int i, j;
    double checksum = 0.0;
    
    /* Initialize with random but reproducible values */
    srand(42);
    for (i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (i = 0; i < 100; i++) {
        arr_s[i].a = rand() % 1000;
        arr_s[i].b = (float)(rand() % 1000) / 10.0f;
    }
    
    for (i = 0; i < 50; i++) {
        arr_h[i].c = rand() % 256;
        arr_h[i].s = rand() % 1000;
        arr_h[i].i = rand() % 1000;
        arr_h[i].l = rand() % 1000;
        arr_h[i].f = (float)(rand() % 1000) / 10.0f;
        arr_h[i].d = (double)(rand() % 1000) / 10.0;
    }
    
    /* ===== PATTERN 1: Simple post-increment in for loop ===== */
    {
        register int* p asm ("r10") = arr_i; /* Encourage register allocation */
        int sum = 0;
        
        for (i = 0; i < 100; i++) {
            sum += *p++;  /* Classic post-increment pattern */
        }
        
        checksum += sum;
        
        /* Compiler barrier via math function */
        checksum += sin(checksum);
    }
    
    /* ===== PATTERN 2: Post-decrement in while loop ===== */
    {
        float* q = &arr_f[99];
        float fsum = 0.0f;
        
        while (q >= arr_f) {
            fsum += *q--;  /* Post-decrement pattern */
        }
        
        checksum += fsum;
        
        /* Reset pointer and use with index */
        q = arr_f;
        for (i = 0; i < 100; i++) {
            fsum += q[i];  /* Array indexing */
        }
        
        checksum += fsum;
        checksum += cos(checksum);
    }
    
    /* ===== PATTERN 3: Nested loops with struct access ===== */
    {
        struct S* ptr = arr_s;
        double ssum = 0.0;
        
        /* Outer loop resets pointer */
        for (j = 0; j < 3; j++) {
            ptr = arr_s;  /* Reset pointer - forces re-evaluation */
            
            /* Inner loop uses pointer increment */
            for (i = 0; i < 50; i++) {
                /* Structure field accesses - decompose to base + offset */
                ssum += ptr->a;  /* First field: offset 0 */
                ssum += ptr->b;  /* Second field: non-zero offset */
                ptr++;           /* Post-increment */
            }
            
            /* Compiler barrier between iterations */
            asm volatile("" ::: "memory");
        }
        
        checksum += ssum;
        checksum += sqrt(fabs(checksum));
    }
    
    /* ===== PATTERN 4: Mixed pointer and index arithmetic ===== */
    {
        int* base = arr_i;
        int idx = 0;
        int msum = 0;
        
        /* Mix pointer and index in same loop */
        for (i = 0; i < 100; i++) {
            /* Different forms of pointer arithmetic */
            msum += *(base + idx);      /* Pointer + index */
            msum += base[idx];          /* Array indexing */
            msum += *(base + 0);        /* Explicit zero offset */
            
            idx++;
            if (idx % 10 == 0) {
                /* Periodic reset to force addressing mode re-evaluation */
                base = &arr_i[idx];
            }
        }
        
        checksum += msum;
    }
    
    /* ===== PATTERN 5: Heterogeneous struct traversal ===== */
    {
        struct Heterogeneous* hptr = arr_h;
        double hsum = 0.0;
        
        #pragma GCC unroll 2
        for (i = 0; i < 50; i++) {
            /* Access different fields with different offsets */
            hsum += hptr->c;  /* char: offset 0 */
            hsum += hptr->s;  /* short: offset 2 (assuming alignment) */
            hsum += hptr->i;  /* int: offset 4 */
            hsum += hptr->l;  /* long: offset 8 */
            hsum += hptr->f;  /* float: offset 16 */
            hsum += hptr->d;  /* double: offset 20 */
            
            hptr++;  /* Post-increment by struct size */
        }
        
        checksum += hsum;
        checksum += sin(checksum * 0.01);
    }
    
    /* ===== PATTERN 6: Compound expressions in loop conditions ===== */
    {
        int index = 0;
        int csum = 0;
        
        /* arr[index++] in condition */
        while (index < 100) {
            csum += arr_i[index++];  /* Post-increment in array access */
        }
        
        index = 100;
        /* arr[--index] in update */
        for (i = 0; i < 100; i++) {
            csum += arr_i[--index];  /* Pre-decrement in array access */
        }
        
        checksum += csum;
    }
    
    /* ===== PATTERN 7: Function call between pointer operations ===== */
    {
        double* dptr = arr_d;
        double dsum = 0.0;
        
        for (i = 0; i < 100; i++) {
            dsum += *dptr;
            dptr++;
            
            /* Function call forces re-evaluation of addressing modes */
            if (i % 17 == 0) {
                dsum += cos(dsum);
                asm volatile("" ::: "memory");
            }
        }
        
        checksum += dsum;
    }
    
    /* ===== PATTERN 8: Restrict-qualified pointers ===== */
    process_with_restrict(arr_i, arr_f, arr_s, 50);
    
    /* ===== PATTERN 9: Noinline helper function ===== */
    checksum += process_subset(arr_i, 100);
    
    /* Final checksum output to prevent dead code elimination */
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
