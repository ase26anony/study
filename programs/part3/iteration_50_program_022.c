/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Heterogeneous structure with different alignments */
struct S {
    char c;
    short s;
    int a;
    float b;
    double d;
    long long ll;
} __attribute__((packed));

/* Helper function that won't be inlined */
__attribute__((noinline)) 
static void process_subset(int *arr, float *farr, struct S *sarr, int n) {
    register int *p = arr;
    register float *q = farr;
    register struct S *r = sarr;
    
    for (int i = 0; i < n; i++) {
        /* Multiple zero-offset accesses through different pointers */
        *p += i;
        *q = sinf(*q);
        r->a += r->c;  /* Base + 0 offset for 'c' (first field) */
        r->b = cosf(r->b);
        
        /* Pointer increment in same statement as access */
        *(p++) += 1;
        *(q++) = sqrtf(*q);
        (r++)->d = (r-1)->d * 2.0;
    }
}

/* Function with aggressive optimization attributes */
__attribute__((optimize("O3")))
static double process_with_auto_inc(int *int_arr, float *float_arr, 
                                   struct S *struct_arr, int size) {
    double checksum = 0.0;
    
    /* Pattern 1: Simple post-increment in for loop */
    {
        register int *p = int_arr;
        for (int i = 0; i < size; i++) {
            checksum += *p++;  /* Should trigger auto-inc */
        }
        
        /* Function call between pointer operations */
        checksum = sin(checksum);
    }
    
    /* Pattern 2: Post-decrement in while loop with reset */
    {
        float *q = float_arr + size - 1;
        int count = size;
        while (count-- > 0) {
            checksum += *q--;  /* Should trigger auto-dec */
            if (count % 10 == 0) {
                /* Compiler barrier via function call */
                checksum = cos(checksum * 0.01);
            }
        }
    }
    
    /* Pattern 3: Nested loops with pointer reset */
    for (int outer = 0; outer < 3; outer++) {
        struct S *ptr = struct_arr;
        for (int inner = 0; inner < size/2; inner++) {
            /* Multiple field accesses with zero offset */
            checksum += ptr->c;    /* offset 0 */
            checksum += ptr->a;    /* offset after c and s */
            checksum += ptr->b;
            checksum += ptr->d;
            
            /* Pointer increment with array-like access */
            (ptr++)->ll = inner * outer;
        }
        
        /* Inline assembly that clobbers memory */
        asm volatile("" ::: "memory");
    }
    
    /* Pattern 4: Mixed pointer and index arithmetic */
    {
        int *base = int_arr;
        register int idx = 0;
        
        #pragma GCC unroll 4
        for (idx = 0; idx < size; idx++) {
            /* Base + index with zero constant part */
            checksum += *(base + idx);
            
            /* Compound expression modifying index */
            float_arr[idx] = (float)(int_arr[idx++] + int_arr[idx]);
            idx--;  /* Reset for next iteration */
        }
    }
    
    /* Pattern 5: Multiple pointer aliases to same array */
    {
        int *p1 = int_arr;
        int *p2 = int_arr + size/2;
        int *p3 = int_arr;
        
        __restrict int *rp1 = int_arr;  /* No aliasing hint */
        __restrict int *rp2 = int_arr + size/3;
        
        for (int i = 0; i < size/4; i++) {
            /* Multiple zero-offset accesses */
            *p1 += *p2;
            *p3 = *p1 * 2;
            
            /* Restrict pointers help alias analysis */
            *rp1++ = *rp2-- + i;
            
            /* Function call between operations */
            if (i % 5 == 0) {
                checksum = sqrt(fabs(checksum));
            }
        }
    }
    
    return checksum;
}

int main(void) {
    /* Arrays of different types and alignments */
    int arr_i[256];
    float arr_f[256];
    double arr_d[256];
    short arr_s[512];
    char arr_c[1024];
    struct S arr_struct[128];
    
    /* Initialize with varying values */
    srand(42);
    for (int i = 0; i < 256; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 512; i++) {
        arr_s[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 1024; i++) {
        arr_c[i] = (char)(rand() % 256);
    }
    
    for (int i = 0; i < 128; i++) {
        arr_struct[i].c = (char)(rand() % 256);
        arr_struct[i].s = (short)(rand() % 1000);
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].d = (double)(rand() % 1000) / 10.0;
        arr_struct[i].ll = (long long)rand() * rand();
    }
    
    /* Process data with various patterns */
    double checksum = 0.0;
    
    /* Loop 1: Simple pointer traversal with post-inc */
    {
        int *p = arr_i;
        for (int i = 0; i < 256; i++) {
            checksum += *p++;
        }
        checksum = sin(checksum);  /* Compiler barrier */
    }
    
    /* Loop 2: Post-decrement with while */
    {
        float *q = arr_f + 255;
        int count = 256;
        while (count-- > 0) {
            checksum += *q--;
            if (count == 128) {
                q = arr_f + 255;  /* Pointer reset */
            }
        }
    }
    
    /* Loop 3: Nested loops with struct access */
    for (int outer = 0; outer < 2; outer++) {
        struct S *ptr = arr_struct;
        for (int inner = 0; inner < 128; inner++) {
            checksum += ptr->a + ptr->b;
            ptr->d = checksum * 0.5;
            ptr++;
        }
    }
    
    /* Loop 4: Mixed index and pointer arithmetic */
    {
        int *base = arr_i;
        for (int i = 0; i < 256; i++) {
            checksum += *(base + i);  /* Base + index */
            
            /* Compound expression in array index */
            arr_f[i] = arr_f[i] + arr_f[i + 1];
            arr_i[i] = arr_i[i] + arr_i[i + 1];
        }
    }
    
    /* Call the optimized function */
    checksum += process_with_auto_inc(arr_i, arr_f, arr_struct, 128);
    
    /* Call the noinline helper */
    process_subset(arr_i, arr_f, arr_struct, 64);
    
    /* Final processing with all array types */
    {
        /* Different pointer types accessing memory */
        char *cp = arr_c;
        short *sp = arr_s;
        int *ip = arr_i;
        float *fp = arr_f;
        double *dp = arr_d;
        
        for (int i = 0; i < 64; i++) {
            /* Multiple zero-offset accesses */
            *cp = (char)(*ip % 256);
            *sp = (short)(*fp * 100);
            *dp = (double)(*ip) + *fp;
            
            /* All pointers increment */
            cp++; sp++; ip++; fp++; dp++;
            
            /* Function call between operations */
            if (i % 8 == 0) {
                checksum = cos(checksum);
            }
        }
    }
    
    /* Prevent dead code elimination */
    volatile double result = checksum;
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
