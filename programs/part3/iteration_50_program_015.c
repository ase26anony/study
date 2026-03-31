/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Compiler hints to influence optimization */
#pragma GCC optimize("O3")
#define RESTRICT __restrict

/* Mixed data types for offset calculation stress */
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
static double process_subset(int* RESTRICT arr_int, float* RESTRICT arr_float, 
                             struct Heterogeneous* RESTRICT arr_struct, int n) {
    double sum = 0.0;
    int* p = arr_int;
    float* q = arr_float;
    struct Heterogeneous* r = arr_struct;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* [reg + 0] addressing */
        sum += *q;      /* Another [reg + 0] */
        sum += r->i;    /* Base + 0 for first struct field */
        p++;
        q++;
        r++;
    }
    
    /* Pattern 2: Post-decrement with while loop */
    while (n-- > 0) {
        sum += *(--p);  /* Pre-decrement in array access */
        sum += *(q--);  /* Post-decrement with pointer dereference */
    }
    
    return sum;
}

/* Mark specific loops for aggressive optimization */
__attribute__((optimize("O3")))
static void process_with_auto_inc(int* RESTRICT base, int size) {
    register int* p asm ("r12");  /* Hint for register allocation */
    int sum = 0;
    
    /* Loop-carried dependency with induction variable */
    for (p = base; p < base + size; p++) {
        sum += *p;
        /* Function call to force re-evaluation of addressing modes */
        if ((p - base) % 16 == 0) {
            sum += (int)sin((double)(p - base));
        }
    }
    
    /* Mixed pointer and index arithmetic */
    int idx = 0;
    int* alias = base;
    while (idx < size) {
        /* [reg + 0] pattern with different pointer */
        sum += *(alias + 0);
        /* Compound expression with index modification */
        sum += base[idx++];
        alias = &base[idx];
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = sum;
    (void)dummy;
}

int main(void) {
    /* Declare arrays of different types and alignments */
    int arr_int[200];
    float arr_float[200];
    double arr_double[200];
    short arr_short[200];
    struct Heterogeneous arr_struct[100];
    
    /* Initialize with varying values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_int[i] = rand() % 1000;
        arr_float[i] = (float)rand() / RAND_MAX;
        arr_double[i] = (double)rand() / RAND_MAX;
        arr_short[i] = (short)(rand() % 1000);
        if (i < 100) {
            arr_struct[i].c = (char)(rand() % 256);
            arr_struct[i].s = (short)(rand() % 1000);
            arr_struct[i].i = rand() % 1000;
            arr_struct[i].l = rand() * 1000L;
            arr_struct[i].f = (float)rand() / RAND_MAX;
            arr_struct[i].d = (double)rand() / RAND_MAX;
        }
    }
    
    double total_sum = 0.0;
    
    /* Pattern A: for loop with post-increment */
    #pragma GCC unroll 4
    for (int* p = arr_int; p < arr_int + 100; /* increment in body */) {
        total_sum += *p++;
        total_sum += *p++;
        total_sum += *p++;
        total_sum += *p++;
    }
    
    /* Compiler barrier via math function */
    total_sum += sin(total_sum);
    
    /* Pattern B: while loop with post-decrement */
    float* q = &arr_float[99];
    int counter = 100;
    while (counter-- > 0) {
        total_sum += *q--;
        /* Nested pointer arithmetic */
        if (counter % 10 == 0) {
            total_sum += *(q + 0);  /* Explicit [reg + 0] */
        }
    }
    
    /* Pattern C: Nested loops with struct access */
    for (int outer = 0; outer < 2; outer++) {
        struct Heterogeneous* ptr = arr_struct;
        for (int inner = 0; inner < 50; inner++) {
            /* Multiple [reg + 0] accesses to different struct fields */
            total_sum += ptr->i;    /* First field, offset 0 */
            total_sum += ptr->c;    /* Also offset 0 (after alignment) */
            total_sum += ptr->s;    /* May have small offset */
            ptr++;  /* Post-increment */
        }
        /* Outer loop resets pointer - tests loop-carried dependencies */
        total_sum += cos(total_sum);  /* Another compiler barrier */
    }
    
    /* Pattern D: Mixed integer index and pointer arithmetic */
    int* base_ptr = arr_int + 100;
    for (int i = 0; i < 100; i++) {
        /* ptr = &base + idx pattern */
        int* current = base_ptr + i;
        total_sum += *current;  /* [reg + 0] with computed base */
        
        /* Compound expression with index modification */
        total_sum += arr_int[i++];  /* arr[index++] in expression */
        total_sum += arr_int[--i];  /* arr[--index] in expression */
        
        /* Inline assembly to clobber memory */
        asm volatile("" ::: "memory");
    }
    
    /* Pattern E: Different pointer types with zero offset */
    {
        int* p1 = arr_int;
        float* p2 = arr_float;
        double* p3 = arr_double;
        short* p4 = arr_short;
        
        for (int i = 0; i < 50; i++) {
            /* Multiple [reg + 0] patterns with different base registers */
            total_sum += *p1;
            total_sum += *p2;
            total_sum += *p3;
            total_sum += *p4;
            
            /* All pointers increment together */
            p1++; p2++; p3++; p4++;
            
            /* Array indexing that may convert to auto-inc */
            total_sum += arr_int[i] + arr_float[i] + arr_double[i];
        }
    }
    
    /* Call helper function for additional find_inc context */
    total_sum += process_subset(arr_int, arr_float, arr_struct, 50);
    
    /* Aggressive optimization on specific loop */
    process_with_auto_inc(arr_int, 200);
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %f\n", total_sum);
    
    return 0;
}
