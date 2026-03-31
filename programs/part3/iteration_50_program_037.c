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
};

/* Helper function marked noinline to create separate context */
__attribute__((noinline)) 
static int process_subset(int* RESTRICT arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Pattern 1: Simple post-increment in while loop */
    while (p < end) {
        sum += *p++;
    }
    
    /* Pattern 2: Post-decrement with reset */
    p = end - 1;
    while (p >= arr) {
        sum += *p--;
    }
    
    return sum;
}

/* Another noinline helper with different type */
__attribute__((noinline))
static float process_floats(float* RESTRICT arr, int n) {
    float sum = 0.0f;
    float* q = arr;
    
    /* Mix pointer and index arithmetic */
    for (int i = 0; i < n; i++) {
        sum += *(q + i);  /* [reg + 0] addressing when q is base */
    }
    
    /* Reset and use pure pointer arithmetic */
    q = arr;
    for (int i = 0; i < n; i++) {
        sum += q[i];  /* Array indexing */
        /* Function call to force re-evaluation */
        sum += sinf(sum);
    }
    
    return sum;
}

int main(void) {
    /* Declare arrays of different types and sizes */
    int arr_i[200];
    float arr_f[200];
    double arr_d[150];
    struct Heterogeneous arr_s[100];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        if (i < 150) arr_d[i] = (double)(rand() % 1000) / 10.0;
        if (i < 100) {
            arr_s[i].c = (char)(rand() % 256);
            arr_s[i].s = (short)(rand() % 1000);
            arr_s[i].i = rand() % 1000;
            arr_s[i].l = rand() % 1000;
            arr_s[i].f = (float)(rand() % 1000) / 10.0f;
            arr_s[i].d = (double)(rand() % 1000) / 10.0;
        }
    }
    
    int total_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* ===== PATTERN 1: Simple post-increment with int pointer ===== */
    {
        register int* p asm ("r12") = arr_i;  /* Hint for register allocation */
        int* end = arr_i + 100;
        
        #pragma GCC unroll 4
        for (; p < end; p++) {
            total_sum += *p;
        }
        
        /* Function call between patterns */
        float_sum += cosf((float)total_sum);
    }
    
    /* ===== PATTERN 2: Post-decrement with float pointer ===== */
    {
        float* q = arr_f + 99;
        float* start = arr_f;
        
        while (q >= start) {
            float_sum += *q--;
            /* Compound expression with index modification */
            if (q > start) {
                float_sum += arr_f[(int)(q - start)];  /* Mixed pointer/index */
            }
        }
        
        /* Reset pointer with different alias */
        float* r = arr_f;
        float* r_end = arr_f + 100;
        while (r < r_end) {
            float_sum += *r++;
            /* Another function call to obscure aliasing */
            float_sum += sinf(*r);
        }
    }
    
    /* ===== PATTERN 3: Nested loops with struct access ===== */
    {
        struct Heterogeneous* ptr = arr_s;
        for (int outer = 0; outer < 2; outer++) {
            /* Inner loop with pointer stepping through struct array */
            for (int i = 0; i < 50; i++) {
                /* Access first field (offset 0) - triggers [reg + 0] */
                total_sum += ptr->i;
                /* Access other fields (non-zero offsets) */
                float_sum += ptr->f;
                double_sum += ptr->d;
                ptr++;
            }
            /* Outer loop resets pointer */
            ptr = arr_s;
            
            /* Inline assembly as compiler barrier */
            asm volatile("" ::: "memory");
        }
    }
    
    /* ===== PATTERN 4: Mixed pointer and index arithmetic ===== */
    {
        int* base = arr_i + 100;  /* Second half of array */
        int idx = 0;
        
        /* Loop with explicit induction variable */
        for (register int i = 0; i < 100; i++) {
            /* Multiple base registers with constant offset 0 */
            int* p1 = base + idx;
            int* p2 = base + idx + 1;
            
            total_sum += *p1;      /* [reg + 0] when p1 is the register */
            total_sum += p2[0];    /* Another [reg + 0] pattern */
            
            idx += 2;
            
            /* Complex expression that might decompose */
            total_sum += *(base + i);  /* [base_reg + i*4] */
        }
    }
    
    /* ===== PATTERN 5: Double pointer with post-increment ===== */
    {
        double* dptr = arr_d;
        double* dend = arr_d + 150;
        
        /* Unrolled loop pragma */
        #pragma GCC unroll 8
        for (; dptr < dend; dptr++) {
            double_sum += *dptr;
            /* Function call between memory ops */
            if ((dptr - arr_d) % 16 == 0) {
                double_sum += sqrt(fabs(*dptr));
            }
        }
    }
    
    /* ===== PATTERN 6: Compound expressions in loop conditions ===== */
    {
        int index = 0;
        int arr_temp[50];
        memcpy(arr_temp, arr_i, 50 * sizeof(int));
        
        /* arr[index++] in condition */
        while (index < 50 && arr_temp[index++] > 0) {
            total_sum += arr_temp[index - 1];
        }
        
        /* arr[--index] in update */
        for (index = 49; index >= 0; ) {
            total_sum += arr_temp[--index];
            /* Mix with pointer access */
            int* tmp_ptr = &arr_temp[index];
            total_sum += *tmp_ptr;  /* [reg + 0] */
        }
    }
    
    /* ===== Call helper functions for additional contexts ===== */
    total_sum += process_subset(arr_i, 100);
    float_sum += process_floats(arr_f, 100);
    
    /* ===== Pattern with multiple zero-offset accesses ===== */
    {
        /* Create multiple pointer aliases to same base */
        int* alias1 = arr_i;
        int* alias2 = arr_i;
        float* alias3 = arr_f;
        float* alias4 = arr_f;
        
        /* All these should generate [reg + 0] addressing */
        total_sum += *alias1;
        total_sum += alias2[0];
        float_sum += *alias3;
        float_sum += alias4[0];
        
        /* Structure with first field access */
        struct Heterogeneous* sptr = arr_s;
        total_sum += sptr->i;  /* First field after char/short padding */
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksums: int=%d, float=%.2f, double=%.2f\n", 
           total_sum, float_sum, double_sum);
    
    return total_sum > 0 ? 0 : 1;
}
