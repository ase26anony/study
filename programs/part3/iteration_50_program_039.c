/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Compiler-specific pragmas and attributes */
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

/* Helper function marked noinline to create separate optimization context */
__attribute__((noinline, optimize("O3")))
static double process_subset(int* RESTRICT arr_int, float* RESTRICT arr_float, 
                             struct Heterogeneous* RESTRICT arr_struct, int n) {
    double sum = 0.0;
    int* p = arr_int;
    float* q = arr_float;
    struct Heterogeneous* r = arr_struct;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *p;          /* [reg + 0] addressing */
        sum += *q;          /* Different register, zero offset */
        sum += r->i;        /* Structure field - base + 0 for first int field */
        p++;
        q++;
        r++;
    }
    
    /* Pattern 2: Post-decrement with while loop */
    while (n-- > 0) {
        sum += *(--p);      /* Pre-decrement in array access */
        sum += *(q--);      /* Post-decrement */
    }
    
    return sum;
}

int main(void) {
    /* Declare arrays of different types and alignments */
    int arr_int[200];
    float arr_float[200];
    double arr_double[200];
    short arr_short[200];
    struct Heterogeneous arr_struct[100];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_int[i] = rand() % 1000;
        arr_float[i] = (float)rand() / RAND_MAX;
        arr_double[i] = (double)rand() / RAND_MAX;
        arr_short[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 100; i++) {
        arr_struct[i].c = (char)(rand() % 256);
        arr_struct[i].s = (short)(rand() % 1000);
        arr_struct[i].i = rand() % 1000;
        arr_struct[i].l = rand() * 1000L;
        arr_struct[i].f = (float)rand() / RAND_MAX;
        arr_struct[i].d = (double)rand() / RAND_MAX;
    }
    
    double total_sum = 0.0;
    
    /* ===== PATTERN 1: Simple post-increment in for loop ===== */
    {
        register int* RESTRICT p = arr_int;  /* register hint */
        int sum = 0;
        
        #pragma GCC unroll 4
        for (int i = 0; i < 200; i++) {
            sum += *p++;    /* Post-increment - should trigger find_inc */
        }
        total_sum += sum;
        
        /* Function call as compiler barrier */
        total_sum += sin(total_sum);
    }
    
    /* ===== PATTERN 2: While loop with post-decrement ===== */
    {
        float* RESTRICT q = &arr_float[199];
        float sum = 0.0f;
        int count = 200;
        
        while (count-- > 0) {
            sum += *q--;    /* Post-decrement */
        }
        total_sum += sum;
        
        /* Another compiler barrier */
        total_sum += cos(total_sum);
    }
    
    /* ===== PATTERN 3: Nested loops with pointer reset ===== */
    {
        double* RESTRICT r = arr_double;
        double sum = 0.0;
        
        /* Outer loop resets pointer */
        for (int outer = 0; outer < 4; outer++) {
            r = arr_double + (outer * 50);
            
            /* Inner loop uses pointer arithmetic */
            for (int inner = 0; inner < 50; inner++) {
                sum += *(r + inner);  /* Pointer + index */
            }
        }
        total_sum += sum;
    }
    
    /* ===== PATTERN 4: Mixed integer index and pointer arithmetic ===== */
    {
        short* RESTRICT base = arr_short;
        int sum = 0;
        
        for (int idx = 0; idx < 200; idx++) {
            /* Compound expression with index modification */
            sum += base[idx];          /* Array indexing */
            sum += *(base + idx);      /* Pointer arithmetic */
            
            /* Inline assembly to clobber memory */
            asm volatile("" ::: "memory");
        }
        total_sum += sum;
        
        total_sum += sqrt(fabs(total_sum));
    }
    
    /* ===== PATTERN 5: Structure array traversal ===== */
    {
        struct Heterogeneous* RESTRICT sptr = arr_struct;
        double sum = 0.0;
        
        /* Access different structure fields */
        for (int i = 0; i < 100; i++) {
            /* Multiple [reg + const] accesses, some with offset 0 */
            sum += sptr->i;        /* Offset 0 for first int field after padding */
            sum += sptr->f;        /* Non-zero offset */
            sum += sptr->d;        /* Different offset */
            sptr++;                /* Pointer increment */
        }
        total_sum += sum;
    }
    
    /* ===== PATTERN 6: Complex loop-carried dependencies ===== */
    {
        int* RESTRICT p1 = arr_int;
        int* RESTRICT p2 = arr_int + 100;
        int sum = 0;
        
        for (int i = 0; i < 100; i++) {
            /* Multiple pointers with arithmetic */
            sum += *p1++;
            sum += *p2++;
            
            /* Conditional that might affect pointer */
            if (sum % 2 == 0) {
                sum += arr_int[--(*p1)];  /* Complex addressing */
            }
        }
        total_sum += sum;
    }
    
    /* ===== PATTERN 7: Call to noinline helper function ===== */
    total_sum += process_subset(arr_int, arr_float, arr_struct, 50);
    
    /* ===== PATTERN 8: Compound expressions in loop conditions ===== */
    {
        int index = 199;
        float sum = 0.0f;
        
        while (index >= 0) {
            /* Compound expression modifying index in access */
            sum += arr_float[index--];      /* Post-decrement in array index */
            sum += arr_float[--index];      /* Pre-decrement */
            
            /* Mix with pointer */
            float* tmp = &arr_float[index];
            sum += *tmp;
        }
        total_sum += sum;
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %f\n", total_sum);
    
    return (total_sum > 0) ? 0 : 1;
}
