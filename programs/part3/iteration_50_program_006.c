/* auto-inc-dec-trigger.c
 * 
 * This program is specifically designed to trigger the uncovered lines
 * 1352-1358 in auto-inc-dec.cc by creating patterns that cause GCC's
 * auto-increment/decrement optimization pass to invoke find_inc(true)
 * with memory instructions having reg1_is_const=true and reg1_val=0.
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

/* Structure for array traversal */
struct S {
    int a;
    float b;
    double c;
};

/* __attribute__((noinline)) helper to create another context */
__attribute__((noinline))
static long process_subset(int* base, int n) {
    register int* p asm("r12") = base;  /* register hint */
    long sum = 0;
    int i;
    
    /* Pattern 1: Simple post-increment with zero offset */
    for (i = 0; i < n; i++) {
        sum += *p;      /* [reg + 0] addressing */
        p++;            /* Post-increment */
    }
    
    /* Insert function call to force re-evaluation */
    if (sum > 1000) {
        sum = (long)sin((double)sum);
    }
    
    return sum;
}

/* Another noinline helper with different pattern */
__attribute__((noinline))
static double traverse_structs(struct S* arr, int count) {
    struct S* ptr = arr;
    double total = 0.0;
    
    /* Access first field (offset 0) with pointer increment */
    while (count-- > 0) {
        total += ptr->a;    /* Base + 0 offset */
        total += ptr->b;
        ptr++;              /* Post-increment by structure size */
    }
    
    return total;
}

int main(void) {
    /* Declare arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    struct S arr_s[100];
    struct Heterogeneous arr_h[50];
    
    int i, j;
    long checksum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Initialize with random but reproducible values */
    srand(42);
    for (i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)rand() / RAND_MAX;
        arr_d[i] = (double)rand() / RAND_MAX;
    }
    
    for (i = 0; i < 100; i++) {
        arr_s[i].a = rand() % 500;
        arr_s[i].b = (float)rand() / RAND_MAX;
        arr_s[i].c = (double)rand() / RAND_MAX;
    }
    
    for (i = 0; i < 50; i++) {
        arr_h[i].c = rand() % 256;
        arr_h[i].s = rand() % 10000;
        arr_h[i].i = rand();
        arr_h[i].l = rand() * 1000L;
        arr_h[i].f = (float)rand() / RAND_MAX;
        arr_h[i].d = (double)rand() / RAND_MAX;
    }
    
    /* PATTERN 1: Simple post-increment with multiple pointers */
    /* This should create [reg + 0] addressing patterns */
    {
        int* p1 = arr_i;
        int* p2 = arr_i + 100;
        int* __restrict rp = arr_i + 50;  /* restrict helps alias analysis */
        
        #pragma GCC unroll 4
        for (i = 0; i < 50; i++) {
            checksum += *p1++;     /* Post-increment */
            checksum += *p2++;     /* Another pointer, same base array */
            checksum += *rp++;     /* Restricted pointer */
        }
        
        /* Function call between patterns */
        fsum = (float)sin((double)checksum);
    }
    
    /* PATTERN 2: Post-decrement in while loop */
    {
        float* q = &arr_f[149];
        int count = 150;
        
        while (count-- > 0) {
            fsum += *q;     /* Memory access */
            q--;            /* Post-decrement */
            
            /* Compound expression with index modification */
            if (count % 10 == 0) {
                fsum += arr_f[count];  /* arr[--count] pattern in disguise */
            }
        }
        
        /* Another compiler barrier */
        dsum = cos((double)fsum);
    }
    
    /* PATTERN 3: Nested loops with pointer reset */
    /* Outer loop resets pointer, inner loop uses auto-inc */
    for (j = 0; j < 5; j++) {
        double* r = arr_d + j * 20;
        
        #pragma GCC unroll 2
        for (i = 0; i < 20; i++) {
            dsum += *r;
            r++;    /* Post-increment in inner loop */
        }
        
        /* Mixed pointer/index arithmetic */
        int* base_ptr = arr_i;
        for (i = 0; i < 10; i++) {
            /* ptr = &base + idx pattern */
            int* temp_ptr = base_ptr + i * 3;
            checksum += *temp_ptr;      /* [reg + 0] */
            checksum += *(temp_ptr + 1); /* [reg + 4] */
            checksum += *(temp_ptr + 2); /* [reg + 8] */
        }
    }
    
    /* PATTERN 4: Structure array traversal with field access */
    {
        struct S* sp = arr_s;
        
        /* Loop accessing first field (offset 0) */
        for (i = 0; i < 100; i++) {
            checksum += sp->a;  /* Base + 0 offset */
            sp++;
        }
        
        /* Reset and access with different pattern */
        sp = arr_s;
        for (i = 0; i < 100; i++) {
            checksum += sp->a + (int)sp->b;
            sp++;  /* Post-increment */
        }
    }
    
    /* PATTERN 5: Mixed integer index and pointer arithmetic */
    {
        int* base = arr_i;
        register int idx asm("r11") = 0;  /* register hint */
        
        for (idx = 0; idx < 100; idx++) {
            /* This should decompose to [reg + 0] after optimization */
            int* current = base + idx;
            checksum += *current;
            
            /* Also try direct indexed access */
            checksum += arr_i[idx];
        }
        
        /* Reverse traversal with pre-decrement pattern */
        for (idx = 99; idx >= 0; idx--) {
            checksum += arr_i[idx];  /* arr[--idx] in other form */
        }
    }
    
    /* PATTERN 6: Heterogeneous struct array with different alignments */
    {
        struct Heterogeneous* hp = arr_h;
        
        for (i = 0; i < 50; i++) {
            /* Multiple [reg + 0] accesses to different fields */
            checksum += hp->c;    /* char at offset 0 */
            checksum += hp->s;    /* short at offset 2 */
            checksum += hp->i;    /* int at offset 4 */
            
            hp++;  /* Post-increment by struct size */
        }
    }
    
    /* Call helper functions to create additional contexts */
    checksum += process_subset(arr_i, 50);
    dsum += traverse_structs(arr_s, 50);
    
    /* Final checksum to prevent dead code elimination */
    checksum += (long)fsum + (long)dsum;
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
