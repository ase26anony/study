#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Heterogeneous structure with mixed alignment */
struct S {
    char c;
    int a;
    float b;
    double d;
    short s;
    long l;
};

/* Helper function marked noinline to create separate optimization context */
__attribute__((noinline)) 
static void process_subset(int* restrict arr, int n, int* out) {
    int sum = 0;
    /* Pattern 1: Pointer post-increment with zero offset */
    int* p = arr;
    int* end = arr + n;
    while (p != end) {
        sum += *p++;  /* Should trigger [reg + 0] addressing */
    }
    
    /* Pattern 2: Mixed pointer/index with constant offset */
    for (int i = 0; i < n; i++) {
        sum += *(arr + i);  /* Base + index, offset 0 when i=0 */
    }
    *out = sum;
}

/* Another noinline helper for float arrays */
__attribute__((noinline))
static float process_floats(float* restrict farr, int n) {
    float sum = 0.0f;
    float* q = farr;
    
    /* Pattern 3: Post-decrement in while loop */
    int count = n;
    while (count-- > 0) {
        sum += *q++;
        /* Insert math call as compiler barrier */
        if (count % 7 == 0) {
            sum += sinf(sum);
        }
    }
    
    /* Pattern 4: Compound expression with index modification */
    int idx = 0;
    while (idx < n) {
        sum += farr[idx++];  /* arr[index++] pattern */
    }
    
    return sum;
}

int main() {
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    char arr_c[200];
    short arr_s[200];
    struct S arr_struct[50];
    
    /* Initialize with varying values */
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (rand() % 1000) / 10.0f;
        arr_d[i] = (rand() % 1000) / 10.0;
        arr_c[i] = rand() % 256;
        arr_s[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 50; i++) {
        arr_struct[i].c = rand() % 256;
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (rand() % 1000) / 10.0f;
        arr_struct[i].d = (rand() % 1000) / 10.0;
        arr_struct[i].s = rand() % 1000;
        arr_struct[i].l = rand() * 1000L;
    }
    
    int total_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* ===== PATTERN SET 1: Basic pointer post-increment ===== */
    {
        int* restrict p1 = arr_i;
        int* restrict p2 = arr_i + 100;
        
        #pragma GCC unroll 4
        for (int i = 0; i < 100; i++) {
            /* Multiple pointers with zero-offset accesses */
            total_sum += *p1++;
            total_sum += *p2++;
        }
    }
    
    /* Compiler barrier via math function */
    float_sum += cosf(total_sum);
    
    /* ===== PATTERN SET 2: Nested loops with pointer reset ===== */
    {
        float* q = arr_f;
        for (int outer = 0; outer < 2; outer++) {
            /* Outer loop resets pointer - forces re-evaluation */
            q = arr_f + (outer * 50);
            
            /* Inner loop with post-increment */
            for (int inner = 0; inner < 50; inner++) {
                float_sum += *q++;
                
                /* Structure field access with zero offset (first field) */
                if (inner < 50) {
                    total_sum += arr_struct[inner].a;  /* Base + 0 offset */
                }
            }
            
            /* Another math call as barrier */
            float_sum = sinf(float_sum);
        }
    }
    
    /* ===== PATTERN SET 3: Mixed pointer/index arithmetic ===== */
    {
        register double* r asm("r12") = arr_d;  /* register hint */
        int idx = 0;
        
        while (idx < 100) {
            /* Pointer arithmetic in access */
            double_sum += *(r + idx);  /* [reg + 0] when idx=0 */
            
            /* Compound expression modifying index */
            double_sum += arr_d[idx++];  /* arr[index++] */
            
            /* Multiple base registers */
            if (idx % 10 == 0) {
                double_sum += *r;  /* Direct dereference - zero offset */
                r++;  /* Pointer update */
            }
        }
    }
    
    /* ===== PATTERN SET 4: Structure array traversal ===== */
    {
        struct S* sp = arr_struct;
        for (int i = 0; i < 50; i++) {
            /* Access different struct fields - all decompose to base + offset */
            total_sum += sp->a;      /* Often offset 0 after alignment */
            float_sum += sp->b;
            double_sum += sp->d;
            total_sum += sp->s;
            total_sum += sp->l;
            
            sp++;  /* Pointer increment by struct size */
        }
    }
    
    /* ===== PATTERN SET 5: Post-decrement patterns ===== */
    {
        short* sptr = arr_s + 199;
        int count = 200;
        
        while (count--) {
            total_sum += *sptr--;  /* Post-decrement */
            
            /* Insert inline assembly to clobber memory */
            asm volatile("" ::: "memory");
        }
    }
    
    /* ===== PATTERN SET 6: Function calls that obscure aliasing ===== */
    {
        int local_sum;
        process_subset(arr_i, 100, &local_sum);
        total_sum += local_sum;
        
        float_sum += process_floats(arr_f, 100);
        
        /* More math calls between pointer operations */
        for (int i = 0; i < 50; i += 2) {
            total_sum += arr_i[i];
            float_sum += sqrtf(fabsf(arr_f[i]));
            total_sum += arr_i[i + 1];
        }
    }
    
    /* ===== PATTERN SET 7: Complex loop with multiple induction variables ===== */
    {
        char* cp = arr_c;
        int* ip = arr_i;
        float* fp = arr_f;
        
        for (int i = 0; i < 100; i++) {
            /* Multiple parallel pointer traversals */
            total_sum += *cp++ + *ip++ + (int)(*fp++);
            
            /* Conditional with pointer arithmetic */
            if (i % 3 == 0) {
                /* Access with computed offset that could be zero */
                total_sum += *(ip - 1);  /* [reg + (-1)*size] */
                fp = arr_f + i;  /* Pointer reassignment */
            }
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksums: int=%d, float=%.2f, double=%.2f\n", 
           total_sum, float_sum, double_sum);
    
    return total_sum > 0 ? 0 : 1;
}
