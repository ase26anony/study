/* Compile with: g++ -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec auto-inc-dec.cc -o auto-inc-dec */
/* Also try: g++ -O3 -funroll-loops -fno-omit-frame-pointer -fno-schedule-insns auto-inc-dec.cc -o auto-inc-dec */
/* For RTL debugging: g++ -O1 -da -fdump-rtl-all -fno-expensive-optimizations auto-inc-dec.cc -o auto-inc-dec */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

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
    char pad[3]; /* Create padding for alignment complexity */
};

/* __attribute__((noinline)) to prevent inlining and create separate context */
__attribute__((noinline)) 
static void process_subset(int* base, int size, int* result) {
    register int* p asm ("r12") = base; /* register hint */
    int* end = base + size;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment with zero offset */
    while (p < end) {
        sum += *p;  /* [reg + 0] addressing */
        p++;        /* Post-increment */
    }
    
    /* Pattern 2: Post-decrement */
    p = end - 1;
    while (p >= base) {
        sum += *p--; /* Post-decrement */
    }
    
    *result = sum;
}

/* Another noinline function with different pattern */
__attribute__((noinline, optimize("O3")))
static void process_with_index(struct S* arr, int n, float* out1, int* out2) {
    struct S* ptr = arr;
    struct S* end = arr + n;
    float sum_f = 0.0f;
    int sum_i = 0;
    
    /* Access structure fields - first field has offset 0 */
    while (ptr < end) {
        sum_i += ptr->a;    /* Base + 0 offset for first field */
        sum_f += ptr->b;    /* Base + 4 offset (assuming int is 4 bytes) */
        ptr++;              /* Auto-increment by sizeof(struct S) */
    }
    
    /* Mixed pointer/index arithmetic */
    for (int i = 0; i < n; i++) {
        struct S* tmp = arr + i;  /* Base + index * sizeof(struct S) */
        sum_i += tmp->a;
    }
    
    *out1 = sum_f;
    *out2 = sum_i;
}

int main() {
    srand(time(NULL));
    
    /* Declare arrays of different types */
    int arr_i[100];
    float arr_f[100];
    double arr_d[100];
    struct S arr_s[50];
    struct Heterogeneous arr_h[25];
    
    /* Initialize with random values */
    for (int i = 0; i < 100; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 50; i++) {
        arr_s[i].a = rand() % 1000;
        arr_s[i].b = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < 25; i++) {
        arr_h[i].c = rand() % 256;
        arr_h[i].s = rand() % 1000;
        arr_h[i].i = rand() % 1000;
        arr_h[i].l = rand() % 1000;
        arr_h[i].f = (float)(rand() % 1000) / 10.0f;
        arr_h[i].d = (double)(rand() % 1000) / 10.0;
    }
    
    int total_sum = 0;
    float total_float = 0.0f;
    double total_double = 0.0;
    
    /* PATTERN 1: Simple post-increment with int pointer */
    {
        int* p = arr_i;
        int* end = arr_i + 100;
        int sum = 0;
        
        #pragma GCC unroll 4
        for (; p < end; p++) {
            sum += *p;  /* Should trigger [reg + 0] addressing */
        }
        total_sum += sum;
        
        /* Compiler barrier via math function */
        total_float += sin(sum % 100);
    }
    
    /* PATTERN 2: Post-decrement with float pointer */
    {
        float* q = arr_f + 99;  /* Start from end */
        float sum_f = 0.0f;
        
        while (q >= arr_f) {
            sum_f += *q--;  /* Post-decrement */
        }
        total_float += sum_f;
        
        /* Another compiler barrier */
        total_double += cos(sum_f);
    }
    
    /* PATTERN 3: Nested loops with pointer reset */
    {
        int* outer_ptr = arr_i;
        for (int outer = 0; outer < 10; outer++) {
            int* inner_ptr = outer_ptr;
            int inner_sum = 0;
            
            /* Inner loop with pointer stepping */
            for (int inner = 0; inner < 10; inner++) {
                inner_sum += *inner_ptr++;
            }
            total_sum += inner_sum;
            
            /* Switch to different base for next outer iteration */
            outer_ptr += 10;
        }
    }
    
    /* PATTERN 4: Mixed integer index and pointer arithmetic */
    {
        int* base = arr_i;
        int sum = 0;
        
        for (int idx = 0; idx < 100; idx++) {
            /* ptr = &base + idx - creates [reg + 0] when idx=0 */
            int* ptr = base + idx;
            sum += *ptr;  /* Access with potential auto-inc/dec */
            
            /* Compound expression with index modification */
            if (idx % 2 == 0) {
                sum += arr_i[idx++];  /* arr[index++] */
            } else {
                sum += arr_i[--idx];  /* arr[--index] */
                idx++; /* Recover */
            }
        }
        total_sum += sum;
        
        /* Compiler barrier */
        total_float += sqrt(fabs(sum));
    }
    
    /* PATTERN 5: Structure array traversal with multiple pointers */
    {
        struct S* ptr1 = arr_s;
        struct S* ptr2 = arr_s;
        struct S* end = arr_s + 50;
        int sum_i = 0;
        float sum_f = 0.0f;
        
        /* Two pointers aliasing the same array */
        while (ptr1 < end) {
            sum_i += ptr1->a;  /* First field - offset 0 */
            sum_f += ptr1->b;
            ptr1++;
            
            /* Second pointer with different access pattern */
            if (ptr2 < end - 1) {
                sum_i += ptr2[1].a;  /* Indexed access */
                ptr2 += 2;
            }
        }
        total_sum += sum_i;
        total_float += sum_f;
    }
    
    /* PATTERN 6: Heterogeneous struct with different alignments */
    {
        struct Heterogeneous* hptr = arr_h;
        long sum_l = 0;
        
        for (int i = 0; i < 25; i++) {
            /* Access different fields with different offsets */
            sum_l += hptr->l;  /* 8-byte aligned (assuming 64-bit) */
            total_float += hptr->f;
            total_double += hptr->d;
            
            /* Pointer arithmetic with mixed types */
            hptr++;
        }
        total_sum += (int)sum_l;
    }
    
    /* PATTERN 7: Restrict pointers to help alias analysis */
    {
        int* __restrict rp1 = arr_i;
        int* __restrict rp2 = arr_i + 50;
        int sum1 = 0, sum2 = 0;
        
        for (int i = 0; i < 50; i++) {
            sum1 += *rp1++;  /* Post-increment with restrict */
            sum2 += *rp2++;  /* Different restrict pointer */
        }
        total_sum += sum1 + sum2;
    }
    
    /* Call noinline helper functions */
    int subset_result = 0;
    process_subset(arr_i, 100, &subset_result);
    total_sum += subset_result;
    
    float struct_float_sum = 0.0f;
    int struct_int_sum = 0;
    process_with_index(arr_s, 50, &struct_float_sum, &struct_int_sum);
    total_float += struct_float_sum;
    total_sum += struct_int_sum;
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: int=%d, float=%.2f, double=%.2f\n", 
           total_sum, total_float, total_double);
    
    return 0;
}
