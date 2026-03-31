/* Compile with: gcc -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec auto-inc-dec.cc -lm */
/* Also try: gcc -O3 -funroll-loops -fno-omit-frame-pointer -fno-schedule-insns auto-inc-dec.cc -lm */
/* For RTL debugging: gcc -O1 -da -fdump-rtl-all -fno-expensive-optimizations auto-inc-dec.cc -lm */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Heterogeneous structure with different sized members */
struct S {
    char c;
    short s;
    int a;
    float b;
    double d;
    long l;
};

/* __attribute__((noinline)) helper to create another context for find_inc */
__attribute__((noinline))
static double process_subset(int* int_arr, float* float_arr, struct S* struct_arr, int n) {
    double sum = 0.0;
    int* p = int_arr;
    float* q = float_arr;
    struct S* r = struct_arr;
    
    /* Pattern 1: Pointer post-increment with zero offset */
    for (int i = 0; i < n/2; i++) {
        sum += *p++;  /* Should trigger [reg + 0] addressing */
    }
    
    /* Pattern 2: Mixed pointer and index with compound expressions */
    for (int i = 0; i < n/2; i++) {
        sum += *(q + i);  /* Base + index */
        sum += r[i].a;    /* Structure field access - first field (offset 0) */
    }
    
    return sum;
}

int main() {
    /* Declare arrays of different types and alignments */
    int arr_i[100] __attribute__((aligned(16)));
    float arr_f[100] __attribute__((aligned(8)));
    double arr_d[100];
    char arr_c[200];
    short arr_s[100];
    struct S arr_struct[50];
    
    /* Initialize with random values */
    for (int i = 0; i < 100; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
        arr_c[i] = (char)(rand() % 256);
        arr_s[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 50; i++) {
        arr_struct[i].c = (char)(rand() % 256);
        arr_struct[i].s = (short)(rand() % 1000);
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].d = (double)(rand() % 1000) / 10.0;
        arr_struct[i].l = rand() % 1000;
    }
    
    double total_sum = 0.0;
    
    /* ===== PATTERN 1: Simple post-increment with int pointer ===== */
    {
        register int* p __asm__("r12") = arr_i;  /* register hint */
        int* end = arr_i + 100;
        
        /* Loop with post-increment - should trigger find_inc */
        while (p < end) {
            total_sum += *p++;  /* Post-increment access */
        }
        
        /* Compiler barrier with function call */
        total_sum += sin(total_sum);
    }
    
    /* ===== PATTERN 2: Post-decrement with float pointer ===== */
    {
        float* q = arr_f + 99;  /* Start from end */
        int count = 100;
        
        /* while loop with post-decrement */
        while (count-- > 0) {
            total_sum += *q--;  /* Post-decrement access */
        }
        
        /* Reset pointer with different alias */
        float* q2 = arr_f;
        float* q3 = q2;  /* Multiple aliases for same base */
        
        /* Access with zero offset through different pointers */
        total_sum += *q2;  /* [reg + 0] */
        total_sum += *q3;  /* Another [reg + 0] from different reg */
        
        total_sum += cos(total_sum);
    }
    
    /* ===== PATTERN 3: Nested loops with structure access ===== */
    {
        struct S* ptr = arr_struct;
        
        /* Outer loop resets pointer */
        for (int outer = 0; outer < 2; outer++) {
            /* Inner loop uses pointer increment */
            #pragma GCC unroll 4
            for (int i = 0; i < 25; i++) {
                /* Structure field accesses - decompose to base + constant offset */
                total_sum += ptr->a;    /* First field, offset 0 */
                total_sum += ptr->b;    /* Second field, constant offset */
                ptr++;
            }
            ptr = arr_struct;  /* Reset for next outer iteration */
        }
    }
    
    /* ===== PATTERN 4: Mixed integer index and pointer arithmetic ===== */
    {
        int* base = arr_i;
        int idx = 0;
        
        /* Mix pointer and index arithmetic */
        for (int i = 0; i < 100; i++) {
            /* Compound expression with index modification */
            total_sum += arr_i[idx++];  /* arr[index++] */
            
            /* Pointer arithmetic in same loop */
            int* temp_ptr = base + i;
            total_sum += *temp_ptr;  /* [reg + 0] after arithmetic */
        }
        
        /* Reverse traversal with pre-decrement */
        idx = 99;
        while (idx >= 0) {
            total_sum += arr_i[--idx];  /* arr[--index] */
        }
        
        total_sum += sqrt(fabs(total_sum));
    }
    
    /* ===== PATTERN 5: Multiple data types with alignment considerations ===== */
    {
        /* char array - potential misalignment */
        char* cp = arr_c;
        for (int i = 0; i < 100; i++) {
            total_sum += cp[i];  /* Byte access */
        }
        
        /* short array - 2-byte alignment */
        short* sp = arr_s;
        for (int i = 0; i < 100; i++) {
            total_sum += *sp++;
        }
        
        /* double array - 8-byte alignment */
        double* dp = arr_d;
        double* __restrict rp = dp;  /* restrict to help alias analysis */
        for (int i = 0; i < 100; i++) {
            total_sum += *rp++;
        }
        
        /* Inline assembly as compiler barrier */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* ===== PATTERN 6: Function call that obscures aliasing ===== */
    {
        /* Call helper function - creates new context for find_inc */
        total_sum += process_subset(arr_i, arr_f, arr_struct, 50);
        
        /* More complex pattern after function call */
        int* p1 = arr_i;
        int* p2 = arr_i + 50;
        
        /* Multiple pointers with arithmetic */
        for (int i = 0; i < 50; i++) {
            total_sum += *(p1 + i);  /* Base + index */
            total_sum += *(p2 - i);  /* Different base */
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %f\n", total_sum);
    
    return 0;
}
