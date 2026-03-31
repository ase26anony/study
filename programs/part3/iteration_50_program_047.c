#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Heterogeneous struct with mixed alignment requirements */
struct MixedData {
    char c;
    short s;
    int i;
    float f;
    double d;
    long long ll;
};

/* Restrict-qualified helper function */
__attribute__((noinline))
static int process_with_restrict(int* __restrict a, float* __restrict b, int n) {
    int sum = 0;
    float prod = 1.0f;
    
    /* Pattern 1: Post-increment with zero offset */
    int* p = a;
    for (int i = 0; i < n; i++) {
        sum += *p++;  /* Should trigger [reg + 0] addressing */
    }
    
    /* Pattern 2: Post-decrement */
    float* q = b + n - 1;
    for (int i = 0; i < n; i++) {
        prod *= *q--;  /* Should trigger [reg + 0] addressing */
    }
    
    /* Mix pointer and index with zero offset */
    int* r = a;
    for (int idx = 0; idx < n; idx++) {
        sum += *(r + 0);  /* Explicit zero offset */
        r++;
    }
    
    return sum + (int)prod;
}

/* Function with attribute to force specific optimization */
__attribute__((optimize("O3")))
static void traverse_struct_array(struct MixedData* arr, int n) {
    /* Multiple base registers accessing same array */
    struct MixedData* ptr1 = arr;
    struct MixedData* ptr2 = arr;
    
    for (int i = 0; i < n; i++) {
        /* Access first field (offset 0) with different pointers */
        ptr1->c = (char)i;
        ptr2->s = (short)i;
        
        /* Access other fields to create different offsets */
        ptr1->i = i * 2;
        ptr1->f = (float)i * 3.14f;
        
        /* Post-increment both pointers */
        ptr1++;
        ptr2++;
    }
}

int main(void) {
    /* Arrays of different types and alignments */
    int arr_i[256];
    float arr_f[256];
    double arr_d[128];
    short arr_s[512];
    struct MixedData arr_mixed[64];
    
    /* Initialize with random but deterministic values */
    srand(42);
    for (int i = 0; i < 256; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
    }
    for (int i = 0; i < 128; i++) {
        arr_d[i] = (double)(rand() % 1000) / 100.0;
    }
    for (int i = 0; i < 512; i++) {
        arr_s[i] = (short)(rand() % 1000);
    }
    for (int i = 0; i < 64; i++) {
        arr_mixed[i].c = (char)(rand() % 256);
        arr_mixed[i].s = (short)(rand() % 1000);
        arr_mixed[i].i = rand() % 10000;
        arr_mixed[i].f = (float)(rand() % 1000) / 10.0f;
        arr_mixed[i].d = (double)(rand() % 1000) / 100.0;
        arr_mixed[i].ll = (long long)rand() * rand();
    }
    
    int total_sum = 0;
    float total_prod = 1.0f;
    
    /* ===== PATTERN 1: Simple post-increment with int* ===== */
    {
        register int* p asm("r12") = arr_i;  /* Hint for register allocation */
        int local_sum = 0;
        
        #pragma GCC unroll 4
        for (int i = 0; i < 256; i++) {
            local_sum += *p++;  /* Post-increment with implied [reg + 0] */
        }
        total_sum += local_sum;
        
        /* Compiler barrier via math function */
        total_prod *= (float)sin((double)local_sum);
    }
    
    /* ===== PATTERN 2: Post-decrement with float* ===== */
    {
        float* q = arr_f + 255;  /* Start at end */
        float local_prod = 1.0f;
        int count = 256;
        
        while (count-- > 0) {
            local_prod *= *q--;  /* Post-decrement */
        }
        total_prod *= local_prod;
        
        /* Another compiler barrier */
        total_sum += (int)cos((double)local_prod);
    }
    
    /* ===== PATTERN 3: Nested loops with pointer reset ===== */
    {
        int* outer_ptr = arr_i;
        
        /* Outer loop resets pointer */
        for (int block = 0; block < 4; block++) {
            int* inner_ptr = outer_ptr;
            
            /* Inner loop uses auto-increment */
            for (int j = 0; j < 64; j++) {
                total_sum += *inner_ptr++;
            }
            
            /* Switch to different base for next iteration */
            outer_ptr += 64;
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
        }
    }
    
    /* ===== PATTERN 4: Mixed pointer/index arithmetic ===== */
    {
        double* base = arr_d;
        
        /* Mix pointer and index in same expression */
        for (int idx = 0; idx < 128; idx++) {
            /* This should decompose to [base_reg + idx*8] then [base_reg + 0] */
            double* ptr = base + idx;
            total_sum += (int)(*ptr * 100.0);
            
            /* Follow with zero-offset access using same pointer */
            volatile double temp = *ptr;  /* [reg + 0] addressing */
            (void)temp;
        }
    }
    
    /* ===== PATTERN 5: Array indexing with auto-inc/dec in expression ===== */
    {
        int index = 511;
        while (index >= 0) {
            /* Compound expression with auto-decrement */
            total_sum += arr_s[index--];  /* arr[--index] pattern */
        }
        
        index = 0;
        while (index < 512) {
            /* Auto-increment in array index */
            total_sum += arr_s[index++];  /* arr[index++] pattern */
        }
    }
    
    /* ===== PATTERN 6: Struct traversal with multiple fields ===== */
    traverse_struct_array(arr_mixed, 64);
    
    /* Access struct fields with pointer arithmetic */
    struct MixedData* mptr = arr_mixed;
    for (int i = 0; i < 64; i++) {
        /* These decompose to [reg + const_offset] where first field is offset 0 */
        total_sum += mptr->i;
        total_prod *= mptr->f;
        mptr++;  /* Post-increment */
    }
    
    /* ===== PATTERN 7: Restrict-qualified function call ===== */
    total_sum += process_with_restrict(arr_i, arr_f, 128);
    
    /* ===== PATTERN 8: Complex expression with embedded auto-inc ===== */
    {
        char* cptr = (char*)arr_i;
        int* iptr = arr_i;
        
        /* Multiple pointers to same data with different types */
        for (int i = 0; i < 256; i++) {
            /* Access through char* (byte offset) */
            total_sum += *cptr;
            cptr += sizeof(int);  /* Manual increment */
            
            /* Access through int* with zero offset */
            total_sum += *iptr;  /* [reg + 0] */
            iptr++;  /* Post-increment */
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %d (float: %f)\n", total_sum, total_prod);
    
    return 0;
}
