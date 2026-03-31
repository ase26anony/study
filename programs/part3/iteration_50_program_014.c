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

/* Noinline helper to create additional context */
__attribute__((noinline))
static double process_subset(int* base, float* fbase, struct S* sbase, int n) {
    double sum = 0.0;
    int* restrict p = base;
    float* restrict q = fbase;
    struct S* restrict r = sbase;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* [reg + 0] addressing */
        sum += *q;      /* Another [reg + 0] */
        sum += r->a;    /* Structure field: base + offset (often 0 for first field after packed) */
        p++;
        q++;
        r++;
    }
    
    /* Pattern 2: Post-decrement with while loop */
    p = &base[n-1];
    while (p >= base) {
        sum += *p--;
    }
    
    return sum;
}

/* Function with aggressive optimization attribute */
__attribute__((optimize("O3")))
static void mixed_access_patterns(int* arr_i, float* arr_f, struct S* arr_s, 
                                  int size_i, int size_f, int size_s) {
    double total = 0.0;
    
    /* Pattern 1: for loop with post-increment */
    register int* p = arr_i;  /* register hint */
    for (int i = 0; i < size_i; i++) {
        total += *p++;  /* Post-increment in same statement as access */
    }
    
    /* Compiler barrier via math function */
    total += sin(total * 0.01);
    
    /* Pattern 2: while loop with post-decrement and reset */
    float* q = arr_f;
    int counter = size_f;
    while (counter-- > 0) {
        total += *q++;
        if (counter % 10 == 0) {
            /* Function call that obscures aliasing */
            total += cos(total * 0.01);
        }
    }
    
    /* Pattern 3: Nested loops with induction variables */
    for (int outer = 0; outer < 3; outer++) {
        struct S* ptr = arr_s;
        for (int inner = 0; inner < size_s; inner++) {
            /* Mixed structure field accesses with different offsets */
            total += ptr->a;    /* May use [reg + const] addressing */
            total += ptr->b;
            total += ptr->d;
            ptr++;  /* Pointer increment */
        }
        
        /* Inline assembly as compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Pattern 4: Mixed pointer and index arithmetic */
    int* base = arr_i;
    for (int idx = 0; idx < size_i; idx++) {
        /* ptr = &base + idx pattern */
        int* ptr = base + idx;
        total += *ptr;  /* [reg + 0] if ptr already includes offset */
        
        /* Compound expression with index modification */
        if (idx % 2 == 0) {
            total += arr_i[idx++];  /* arr[index++] in same expression */
        }
    }
    
    printf("Mixed patterns sum: %f\n", total);
}

int main(void) {
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    short arr_s[200];
    struct S arr_struct[100];
    
    /* Initialize with random values */
    srand(42);
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
        arr_d[i] = (double)(rand() % 1000) / 10.0;
        arr_s[i] = (short)(rand() % 1000);
    }
    
    for (int i = 0; i < 100; i++) {
        arr_struct[i].c = (char)(rand() % 256);
        arr_struct[i].s = (short)(rand() % 1000);
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].d = (double)(rand() % 1000) / 10.0;
        arr_struct[i].ll = (long long)rand() * rand();
    }
    
    double checksum = 0.0;
    
    /* Loop 1: Simple post-increment with restrict */
    {
        int* restrict p1 = arr_i;
        #pragma GCC unroll 4
        for (int i = 0; i < 200; i++) {
            checksum += *p1++;  /* Should trigger auto-inc optimization */
        }
    }
    
    /* Compiler barrier */
    checksum += sin(checksum);
    
    /* Loop 2: Post-decrement with while */
    {
        float* q1 = &arr_f[199];
        int count = 200;
        while (count-- > 0) {
            checksum += *q1--;  /* Post-decrement pattern */
        }
    }
    
    /* Loop 3: Nested loops with pointer reset */
    for (int outer = 0; outer < 5; outer++) {
        struct S* sptr = arr_struct;
        for (int inner = 0; inner < 100; inner++) {
            /* Structure field accesses - different constant offsets */
            checksum += sptr->a;  /* Base + offset */
            checksum += sptr->b;
            sptr++;  /* Pointer increment */
        }
        
        /* Function call between loops to force re-evaluation */
        checksum += cos(checksum * 0.001);
    }
    
    /* Loop 4: Mixed index and pointer arithmetic */
    {
        int* base_ptr = arr_i;
        for (int j = 0; j < 200; j++) {
            /* Create [reg + 0] addressing opportunity */
            int* alias_ptr = base_ptr + j;
            checksum += *alias_ptr;  /* Zero offset from alias_ptr */
            
            /* Compound expression with index modification */
            if (j % 3 == 0) {
                checksum += arr_i[--j];  /* arr[--index] pattern */
                j++;  /* Restore */
            }
        }
    }
    
    /* Loop 5: Different data types with alignment considerations */
    {
        char* cptr = (char*)arr_i;
        short* sptr = arr_s;
        double* dptr = arr_d;
        
        for (int i = 0; i < 100; i++) {
            /* Mixed type accesses - different offsets and alignments */
            checksum += *(int*)(cptr + i*4);  /* Potential misalignment */
            checksum += *sptr++;
            checksum += *dptr++;
        }
    }
    
    /* Call helper function for additional context */
    checksum += process_subset(arr_i, arr_f, arr_struct, 50);
    
    /* Call function with optimization attribute */
    mixed_access_patterns(arr_i, arr_f, arr_struct, 200, 200, 100);
    
    printf("Final checksum: %f\n", checksum);
    
    return 0;
}
