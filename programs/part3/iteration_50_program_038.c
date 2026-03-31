/* Compile with: g++ -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec auto-inc-dec-test.cc -o auto-inc-dec-test -lm */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#pragma GCC optimize("O3")

/* Heterogeneous structure with different alignments */
struct MixedData {
    char c;
    short s;
    int i;
    float f;
    long l;
    double d;
    char padding[7];
};

/* Helper function marked noinline to create separate optimization context */
__attribute__((noinline))
static void process_subset(int* restrict arr, int size, int* restrict out) {
    register int* p = arr;
    register int* end = arr + size;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment in while loop */
    while (p < end) {
        sum += *p++;
    }
    
    /* Pattern 2: Post-decrement with zero offset access */
    p = end - 1;
    while (p >= arr) {
        sum += *p--;
    }
    
    *out = sum;
}

/* Another noinline helper with different data type */
__attribute__((noinline))
static float process_floats(float* restrict arr, int size) {
    register float* q = arr;
    register float* end = arr + size;
    float sum = 0.0f;
    
    /* Mixed pointer and index arithmetic */
    for (int i = 0; i < size; i++) {
        sum += *(q + i);  /* [reg + 0] addressing when i=0 */
    }
    
    /* Reset and use pure pointer arithmetic */
    q = arr;
    while (q < end) {
        sum += *q++;
        /* Function call to force re-evaluation of addressing */
        sum += sinf(sum);
    }
    
    return sum;
}

int main() {
    srand(time(NULL));
    
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    struct MixedData arr_s[100];
    
    /* Initialize with random data */
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 100; i++) {
        arr_s[i].c = rand() % 256;
        arr_s[i].s = rand() % 1000;
        arr_s[i].i = rand() % 1000;
        arr_s[i].f = (float)(rand() % 1000) / 10.0f;
        arr_s[i].l = rand() * 1000L;
        arr_s[i].d = (double)(rand() % 1000) / 10.0;
    }
    
    int total_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* PATTERN 1: Simple post-increment with int pointer */
    #pragma GCC unroll 4
    for (register int* p = arr_i; p < arr_i + 100; /* increment in body */) {
        total_sum += *p++;  /* Post-increment access */
        
        /* Alternate with [reg + 0] addressing */
        if (p == arr_i + 50) {
            total_sum += *p;  /* Zero offset access */
        }
    }
    
    /* Function call barrier */
    float_sum += cosf(total_sum);
    
    /* PATTERN 2: Post-decrement with float pointer */
    register float* q = arr_f + 199;
    register float* start = arr_f;
    
    while (q >= start) {
        float_sum += *q--;  /* Post-decrement access */
        
        /* Insert occasional zero-offset access */
        if (q == arr_f + 100) {
            float_sum += *q;  /* [reg + 0] */
        }
    }
    
    /* PATTERN 3: Nested loops with struct access */
    for (int outer = 0; outer < 10; outer++) {
        register struct MixedData* sptr = arr_s;
        for (int inner = 0; inner < 10; inner++) {
            /* Access different struct fields with constant offsets */
            total_sum += sptr->i;    /* Base + constant offset (often 0 for first int field) */
            float_sum += sptr->f;    /* Base + different constant offset */
            double_sum += sptr->d;   /* Base + another constant offset */
            sptr++;                  /* Pointer increment */
        }
        
        /* Function call between pointer operations */
        double_sum += sin(double_sum);
    }
    
    /* PATTERN 4: Mixed pointer and index arithmetic */
    register double* r = arr_d;
    for (register int idx = 0; idx < 200; idx++) {
        /* Two addressing modes in same loop */
        double_sum += *(r + idx);    /* [reg + idx*8] addressing */
        
        if (idx % 2 == 0) {
            double_sum += *r;        /* [reg + 0] addressing */
        }
        
        /* Compound expression with index modification */
        if (idx == 100) {
            double_sum += arr_d[idx++];  /* arr[--index] pattern */
        }
    }
    
    /* PATTERN 5: Multiple pointer aliases with zero offset */
    int* alias1 = arr_i;
    int* alias2 = arr_i + 50;
    int* alias3 = arr_i + 100;
    
    for (int i = 0; i < 50; i++) {
        /* Multiple [reg + 0] accesses from different base registers */
        total_sum += *alias1;
        total_sum += *alias2;
        total_sum += *alias3;
        
        alias1++;
        alias2++;
        alias3++;
        
        /* Inline assembly to clobber memory */
        asm volatile("" ::: "memory");
    }
    
    /* PATTERN 6: Loop-carried dependencies with reset */
    for (int cycle = 0; cycle < 5; cycle++) {
        register short* sp = (short*)arr_i;  /* Different type, different alignment */
        for (int i = 0; i < 100; i++) {
            total_sum += *sp++;
            
            /* Compound index modification */
            if (i % 10 == 0) {
                total_sum += *(sp + (i % 3));  /* Variable offset */
            }
        }
        
        /* Reset pointer for next cycle */
        sp = (short*)arr_i;
        for (int i = 99; i >= 0; i--) {
            total_sum += *sp--;
        }
    }
    
    /* Call helper functions for additional contexts */
    int subset_sum = 0;
    process_subset(arr_i + 150, 50, &subset_sum);
    total_sum += subset_sum;
    
    float_sum += process_floats(arr_f + 50, 100);
    
    /* PATTERN 7: Complex expression with post-increment in condition */
    int* complex_p = arr_i;
    int complex_idx = 0;
    while (complex_idx < 200) {
        /* Compound expression in condition */
        if ((total_sum += arr_i[complex_idx++]) > 1000000) {
            break;
        }
        
        /* Mixed pointer/index access */
        total_sum += *(complex_p++);
        
        /* Function call barrier */
        if (complex_idx % 33 == 0) {
            float_sum += sqrtf(fabsf(float_sum));
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksums:\n");
    printf("  Integer sum: %d\n", total_sum);
    printf("  Float sum: %f\n", float_sum);
    printf("  Double sum: %lf\n", double_sum);
    printf("  Combined: %lf\n", total_sum + float_sum + double_sum);
    
    return 0;
}
