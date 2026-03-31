/* Compile with: gcc -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec auto-inc-dec-test.c -lm */
/* Also try: gcc -O3 -funroll-loops -fno-omit-frame-pointer -fno-schedule-insns auto-inc-dec-test.c -lm */
/* For RTL debugging: gcc -O1 -da -fdump-rtl-all -fno-expensive-optimizations auto-inc-dec-test.c -lm */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Heterogeneous struct with mixed alignment requirements */
struct MixedData {
    char c;      /* 1 byte */
    short s;     /* 2 bytes (may cause 2-byte alignment) */
    int i;       /* 4 bytes */
    float f;     /* 4 bytes */
    double d;    /* 8 bytes */
    long long l; /* 8 bytes */
};

/* Helper function marked noinline to create separate optimization context */
__attribute__((noinline)) 
static long process_subset(int* base, int n) {
    register int* p asm("r12") = base; /* register hint */
    long sum = 0;
    int i;
    
    /* Pattern: [reg + 0] addressing with pointer increment */
    for (i = 0; i < n; i++) {
        sum += *p;  /* Base + 0 offset */
        p++;        /* Post-increment */
    }
    
    /* Mixed pointer/index arithmetic */
    int* q = base + n/2;
    for (i = 0; i < n/2; i++) {
        sum += *(q + i);  /* [reg + reg*scale] pattern */
    }
    
    return sum;
}

/* Another noinline helper with float operations */
__attribute__((noinline))
static double process_floats(float* arr, int size) {
    float* __restrict rptr = arr;  /* restrict helps alias analysis */
    double sum = 0.0;
    
    /* Post-decrement pattern */
    while (size-- > 0) {
        sum += *rptr++;
        if (size % 10 == 0) {
            /* Function call between pointer ops */
            sum += sin(sum);
        }
    }
    
    return sum;
}

int main() {
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    struct MixedData arr_s[100];
    
    int i, j;
    long total_sum = 0;
    double float_sum = 0.0;
    double double_sum = 0.0;
    
    /* Initialize with random but reproducible values */
    srand(42);
    for (i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 5.0;
    }
    
    for (i = 0; i < 100; i++) {
        arr_s[i].c = (char)(rand() % 256);
        arr_s[i].s = (short)(rand() % 1000);
        arr_s[i].i = rand() % 1000;
        arr_s[i].f = (float)(rand() % 1000) / 10.0f;
        arr_s[i].d = (double)(rand() % 1000) / 5.0;
        arr_s[i].l = (long long)rand() * rand();
    }
    
    /* ===== PATTERN 1: Simple post-increment with int pointer ===== */
    {
        int* p = arr_i;
        int* end = arr_i + 100;
        
        #pragma GCC unroll 4
        for (; p < end; ) {
            total_sum += *p++;  /* Post-increment in access */
        }
        
        /* Function call between patterns */
        float_sum += cos(total_sum);
    }
    
    /* ===== PATTERN 2: Post-decrement with float pointer ===== */
    {
        float* q = arr_f + 99;  /* Start at end */
        int count = 100;
        
        while (count-- > 0) {
            float_sum += *q--;  /* Post-decrement */
        }
        
        /* Reset pointer with different base */
        q = arr_f + 50;
        for (i = 0; i < 50; i++) {
            float_sum += q[i];  /* Indexed access */
        }
    }
    
    /* ===== PATTERN 3: Nested loops with struct pointer ===== */
    {
        struct MixedData* ptr = arr_s;
        
        /* Outer loop resets pointer */
        for (j = 0; j < 3; j++) {
            ptr = arr_s + j * 20;
            
            /* Inner loop with pointer increment */
            for (i = 0; i < 20; i++) {
                /* Multiple field accesses with [reg + const] addressing */
                total_sum += ptr->i;    /* Base + offset for i field */
                float_sum += ptr->f;    /* Base + offset for f field */
                ptr++;                  /* Post-increment */
            }
            
            /* Compiler barrier via function call */
            double_sum += sin(float_sum);
        }
    }
    
    /* ===== PATTERN 4: Mixed pointer/index arithmetic ===== */
    {
        int* base = arr_i + 100;
        int idx = 0;
        
        /* Mix pointer and index in same loop */
        for (i = 0; i < 50; i++) {
            /* Different addressing modes */
            total_sum += *(base + idx);  /* [reg + reg*scale] */
            idx++;
            
            if (i % 5 == 0) {
                /* Switch to pure pointer */
                int* tmp = base + idx;
                total_sum += *tmp;  /* [reg + 0] */
                
                /* Inline assembly clobbers memory */
                asm volatile("" ::: "memory");
            }
        }
    }
    
    /* ===== PATTERN 5: Double pointer with alignment ===== */
    {
        double* r = arr_d;
        double* r_end = arr_d + 150;
        
        /* Unroll pragma to encourage aggressive optimization */
        #pragma GCC unroll 2
        while (r < r_end) {
            /* Access with potential auto-inc/dec */
            double_sum += *r++;
            
            /* Every 8th iteration, do a function call */
            if ((r - arr_d) % 8 == 0) {
                double_sum = sqrt(fabs(double_sum));
            }
        }
    }
    
    /* ===== PATTERN 6: Multiple aliases to same array ===== */
    {
        int* p1 = arr_i;
        int* p2 = arr_i;  /* Another alias */
        int* p3 = arr_i;  /* Third alias */
        
        /* Different pointers accessing same memory with zero offset */
        for (i = 0; i < 10; i++) {
            total_sum += *p1;  /* [reg1 + 0] */
            total_sum += *p2;  /* [reg2 + 0] */
            total_sum += *p3;  /* [reg3 + 0] */
            
            p1 += 3;
            p2 += 5;
            p3 += 7;
        }
    }
    
    /* ===== PATTERN 7: Complex expressions with index modification ===== */
    {
        int index = 0;
        
        /* Compound expressions in loop condition */
        while (index < 50) {
            total_sum += arr_i[index++];  /* arr[index++] */
            total_sum += arr_i[--index];  /* arr[--index] */
            index += 2;
            
            /* Array of shorts for different scale factor */
            short* sp = (short*)arr_i;
            total_sum += sp[index * 2];  /* Different scale: [reg + reg*2] */
        }
    }
    
    /* Call helper functions for additional contexts */
    total_sum += process_subset(arr_i + 150, 25);
    float_sum += process_floats(arr_f + 150, 25);
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksums: %ld, %f, %f\n", 
           total_sum, float_sum, double_sum);
    
    return (int)(total_sum + float_sum + double_sum) % 100;
}
