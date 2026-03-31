#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")
#pragma GCC push_options

/* Heterogeneous structure with mixed alignment */
struct MixedData {
    char c;
    short s;
    int i;
    float f;
    double d;
    long long ll;
} __attribute__((packed));

/* Another structure with zero-offset first field */
struct ZeroOffset {
    int first;      /* This will be at offset 0 */
    float second;
    char third;
};

/* Restrict-qualified pointer helper */
__attribute__((noinline, optimize("O3")))
static int process_restricted(int* __restrict p, int* __restrict q, int n) {
    int sum = 0;
    /* Pattern: [reg + 0] addressing with multiple base registers */
    for (int i = 0; i < n; ++i) {
        sum += p[0];  /* Zero offset access */
        sum += q[0];  /* Another zero offset access */
        p++;  /* Post-increment */
        q++;  /* Post-increment */
    }
    return sum;
}

/* Function with mixed pointer/index patterns */
__attribute__((noinline))
static double traverse_with_barrier(double* base, int size) {
    double sum = 0.0;
    double* p = base;
    int idx = 0;
    
    /* Mixed pointer and index arithmetic */
    while (idx < size) {
        /* Access with zero offset via pointer */
        sum += *p;  /* This becomes [reg + 0] */
        
        /* Compiler barrier via math function */
        sum = sin(sum * 0.01);
        
        /* Pointer arithmetic with constant 0 offset */
        p = base + idx;  /* Reset to [base + idx*8] */
        idx++;
        
        /* Another zero-offset access */
        if (idx < size) {
            sum += p[0];  /* Zero offset */
        }
    }
    return sum;
}

/* Nested loops with induction variables */
__attribute__((optimize("O3")))
static long long process_structs(struct MixedData* data, int count) {
    long long total = 0;
    struct MixedData* ptr = data;
    
    /* Outer loop resets pointer */
    for (int outer = 0; outer < 3; ++outer) {
        ptr = data;  /* Reset to base */
        
        /* Inner loop with post-increment */
        for (int i = 0; i < count; ++i) {
            /* Multiple zero-offset accesses to different types */
            total += ptr->c;  /* First field at offset 0 */
            total += (int)ptr->f;
            
            /* Post-increment on struct pointer */
            ptr++;
        }
        
        /* Compiler barrier */
        total = llabs(total);
    }
    return total;
}

int main(void) {
    /* Seed for reproducible values */
    srand(42);
    
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    struct MixedData arr_mixed[100];
    struct ZeroOffset arr_zero[100];
    
    /* Initialize with varying values */
    for (int i = 0; i < 200; ++i) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 100; ++i) {
        arr_mixed[i].c = rand() % 256;
        arr_mixed[i].s = rand() % 1000;
        arr_mixed[i].i = rand();
        arr_mixed[i].f = (float)rand() / RAND_MAX;
        arr_mixed[i].d = (double)rand() / RAND_MAX;
        arr_mixed[i].ll = (long long)rand() * rand();
        
        arr_zero[i].first = rand() % 1000;
        arr_zero[i].second = (float)(rand() % 1000) / 10.0f;
        arr_zero[i].third = rand() % 256;
    }
    
    int checksum = 0;
    double fp_checksum = 0.0;
    
    /* PATTERN 1: Simple post-increment with int pointer */
    {
        register int* p asm ("r12") = arr_i;  /* Hint for register allocation */
        for (int i = 0; i < 100; ++i) {
            checksum += *p++;  /* Post-increment in access */
        }
        
        /* Compiler barrier */
        fp_checksum += sin(checksum * 0.001);
    }
    
    /* PATTERN 2: Post-decrement with float pointer */
    {
        float* q = &arr_f[99];
        int counter = 100;
        while (counter-- > 0) {
            fp_checksum += *q--;  /* Post-decrement */
            
            /* Mix with zero-offset access */
            if (counter % 10 == 0) {
                fp_checksum += q[0];  /* Zero offset */
            }
        }
    }
    
    /* PATTERN 3: Nested loops with struct pointer */
    {
        struct ZeroOffset* sptr = arr_zero;
        for (int outer = 0; outer < 5; ++outer) {
            /* Reset pointer for each outer iteration */
            sptr = arr_zero;
            
            for (int inner = 0; inner < 50; ++inner) {
                /* Zero-offset access to first field */
                checksum += sptr->first;  /* Offset 0 */
                checksum += sptr->third;
                
                /* Post-increment */
                sptr++;
            }
            
            /* Barrier function call */
            fp_checksum = cos(fp_checksum);
        }
    }
    
    /* PATTERN 4: Mixed pointer and index arithmetic */
    {
        double* base = arr_d;
        for (int i = 0; i < 100; ++i) {
            /* Pointer arithmetic in access */
            fp_checksum += *(base + i);  /* [base + i*8] */
            
            /* Also access with zero offset */
            if (i % 7 == 0) {
                double* tmp = base + i;
                fp_checksum += tmp[0];  /* Zero offset */
            }
        }
        
        /* Inline assembly to clobber memory */
        asm volatile("" ::: "memory");
    }
    
    /* PATTERN 5: Array indexing with auto-increment in index */
    {
        int idx = 0;
        while (idx < 100) {
            /* Compound expression with index modification */
            checksum += arr_i[idx++];  /* Post-increment in array access */
            checksum += arr_i[--idx];  /* Pre-decrement in array access */
            idx++;
            
            /* Barrier */
            if (idx % 20 == 0) {
                fp_checksum = sqrt(fabs(fp_checksum));
            }
        }
    }
    
    /* PATTERN 6: Restricted pointer function */
    checksum += process_restricted(arr_i, &arr_i[100], 50);
    
    /* PATTERN 7: Struct traversal with mixed access */
    checksum += process_structs(arr_mixed, 50);
    
    /* PATTERN 8: Traversal with math barriers */
    fp_checksum += traverse_with_barrier(arr_d, 100);
    
    /* Final computation to prevent elimination */
    printf("Integer checksum: %d\n", checksum);
    printf("Floating checksum: %f\n", fp_checksum);
    
    /* Use results to prevent dead code elimination */
    if (checksum > 0 && fp_checksum > 0) {
        return 0;
    }
    return 1;
}

#pragma GCC pop_options
