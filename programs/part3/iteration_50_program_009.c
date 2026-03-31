/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")

/* Heterogeneous structure with mixed alignment requirements */
struct MixedData {
    char c;
    short s;
    int i;
    float f;
    double d;
    long l;
    char pad[3];
} __attribute__((packed));

/* Restrict-qualified helper function */
__attribute__((noinline, optimize("O3")))
static double process_restricted(int* __restrict p1, float* __restrict p2, 
                                 struct MixedData* __restrict p3, int n) {
    double sum = 0.0;
    register int* rp = p1;  /* register hint for induction variable */
    register float* rf = p2;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *rp;      /* [reg + 0] addressing */
        sum += *rf;      /* Another [reg + 0] with different base */
        rp++;
        rf++;
    }
    
    /* Pattern 2: Structure field access (base + constant offset, often 0) */
    for (int i = 0; i < n/2; i++) {
        sum += p3[i].c;  /* First field, offset 0 */
        sum += p3[i].s;
        sum += p3[i].i;
        sum += p3[i].f;
        sum += p3[i].d;
        sum += p3[i].l;
    }
    
    return sum;
}

int main(void) {
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    short arr_s[200];
    char arr_c[200];
    struct MixedData arr_md[100];
    
    /* Initialize with varying values */
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (rand() % 1000) / 10.0f;
        arr_d[i] = (rand() % 1000) / 10.0;
        arr_s[i] = rand() % 1000;
        arr_c[i] = rand() % 256;
    }
    
    for (int i = 0; i < 100; i++) {
        arr_md[i].c = rand() % 256;
        arr_md[i].s = rand() % 1000;
        arr_md[i].i = rand() % 1000;
        arr_md[i].f = (rand() % 1000) / 10.0f;
        arr_md[i].d = (rand() % 1000) / 10.0;
        arr_md[i].l = rand() % 1000;
    }
    
    double total_sum = 0.0;
    
    /* PATTERN 1: Post-increment in for loop with int* */
    #pragma GCC unroll 4
    for (int* p = arr_i; p < &arr_i[100]; ) {
        total_sum += *p++;  /* Post-increment in same statement as access */
        total_sum += *p++;
        total_sum += *p++;
        total_sum += *p++;
    }
    
    /* Compiler barrier with function call */
    total_sum += sin(total_sum);
    
    /* PATTERN 2: Post-decrement in while loop with float* */
    float* q = &arr_f[99];
    while (q >= arr_f) {
        total_sum += *q--;  /* Post-decrement */
        /* Force re-evaluation of addressing mode */
        asm volatile("" : : "r"(q) : "memory");
    }
    
    /* PATTERN 3: Mixed pointer and index arithmetic */
    for (int idx = 0; idx < 100; idx++) {
        /* Multiple base registers with zero offset */
        total_sum += *(arr_i + idx);    /* [base + idx*4] */
        total_sum += *(arr_f + idx);    /* Different base register */
        total_sum += *(arr_d + idx);    /* Another base register */
        
        /* Compound expressions with index modification */
        total_sum += arr_s[idx++];      /* idx modified in access */
        total_sum += arr_c[--idx];      /* idx modified back */
    }
    
    total_sum += cos(total_sum);
    
    /* PATTERN 4: Nested loops with pointer reset */
    for (int outer = 0; outer < 10; outer++) {
        register double* r = arr_d;  /* register hint */
        for (int inner = 0; inner < 20; inner++) {
            total_sum += *r++;
            /* Access with zero offset from different base */
            total_sum += arr_i[inner];
        }
        /* Outer loop resets pointer - creates new induction variable */
    }
    
    /* PATTERN 5: Structure array traversal with pointer */
    struct MixedData* md_ptr = arr_md;
    for (int i = 0; i < 50; i++) {
        /* Structure field accesses - decompose to base + constant offset */
        total_sum += md_ptr->c;  /* offset 0 */
        total_sum += md_ptr->s;
        total_sum += md_ptr->i;
        total_sum += md_ptr->f;
        total_sum += md_ptr->d;
        total_sum += md_ptr->l;
        md_ptr++;  /* Pointer increment */
    }
    
    /* PATTERN 6: Complex expression with array indexing */
    int index = 0;
    while (index < 100) {
        /* arr[index++] and arr[--index] in same expression */
        total_sum += arr_i[index++] * arr_i[--index];
        index++;  /* Separate increment to create dependency */
        
        /* Pointer arithmetic with index */
        total_sum += *(arr_f + index);
        total_sum += *(arr_d + index * 2);
    }
    
    /* Call helper function with restrict qualifiers */
    total_sum += process_restricted(arr_i + 50, arr_f + 50, arr_md + 25, 25);
    
    /* Final compiler barrier */
    asm volatile("" : : "r"(total_sum) : "memory");
    
    /* Prevent dead code elimination */
    printf("Checksum: %f\n", total_sum);
    
    return 0;
}
