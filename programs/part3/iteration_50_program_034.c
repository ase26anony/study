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
} __attribute__((packed));

/* Noinline helper to create another context */
__attribute__((noinline)) 
static int process_subset(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Pattern: [reg + 0] addressing with post-increment */
    while (p < end) {
        sum += *p++;  /* Should trigger find_inc with reg1_val = 0 */
    }
    
    /* Mixed pointer/index arithmetic */
    for (int i = 0; i < n/2; i++) {
        sum += *(arr + i);  /* Base + index */
    }
    
    return sum;
}

/* Function with aggressive optimization */
__attribute__((optimize("O3")))
static double process_floats(float *arr, int n) {
    double sum = 0.0;
    register float *q __asm__("r12") = arr;  /* Hint for register allocation */
    float *end = arr + n;
    
    /* Reverse traversal with post-decrement */
    q = end - 1;
    while (q >= arr) {
        sum += *q--;  /* Post-decrement pattern */
        /* Function call to force re-evaluation of addressing */
        sum += sin(sum * 0.01);
    }
    
    return sum;
}

int main(void) {
    /* Arrays of different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[100];
    char arr_c[400];
    short arr_s[200];
    struct S arr_struct[50];
    
    /* Initialize with varying values */
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 1000;
        arr_f[i] = (rand() % 1000) * 0.1f;
        arr_s[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 100; i++) {
        arr_d[i] = (rand() % 1000) * 0.01;
    }
    
    for (int i = 0; i < 400; i++) {
        arr_c[i] = rand() % 256;
    }
    
    for (int i = 0; i < 50; i++) {
        arr_struct[i].c = rand() % 256;
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (rand() % 1000) * 0.1f;
        arr_struct[i].d = (rand() % 1000) * 0.01;
        arr_struct[i].s = rand() % 1000;
    }
    
    int total_sum = 0;
    double float_sum = 0.0;
    
    /* PATTERN 1: Simple post-increment with int pointer */
    #pragma GCC unroll 4
    for (int *p = arr_i; p < arr_i + 100; /* increment in body */) {
        total_sum += *p++;  /* Post-increment in access */
        
        /* Alternate with compound expression */
        if (p < arr_i + 50) {
            total_sum += arr_i[(p - arr_i)++];  /* arr[index++] pattern */
        }
    }
    
    /* Compiler barrier via math function */
    float_sum += cos(total_sum * 0.001);
    
    /* PATTERN 2: Multiple pointer aliases with zero offset */
    int *alias1 = arr_i + 50;
    int *alias2 = arr_i + 50;  /* Same base, will generate [reg + 0] */
    float *alias3 = arr_f + 75;
    float *alias4 = arr_f + 75;
    
    for (int i = 0; i < 25; i++) {
        /* Multiple [reg + 0] accesses */
        total_sum += *alias1;
        float_sum += *alias3;
        
        /* Update pointers separately */
        alias1++;
        alias3++;
        
        /* Access through second alias */
        total_sum += *alias2 * 2;
        float_sum += *alias4 * 0.5f;
        
        alias2++;
        alias4++;
    }
    
    /* PATTERN 3: Nested loops with pointer reset */
    for (int outer = 0; outer < 3; outer++) {
        int *ptr = arr_i + 10 * outer;
        
        /* Inner loop with pointer traversal */
        for (int inner = 0; inner < 20; inner++) {
            total_sum += *ptr++;
            
            /* Mix with index variable */
            total_sum += *(arr_i + inner);  /* [base + index] */
        }
        
        /* Function call between pointer operations */
        float_sum += sin(float_sum);
    }
    
    /* PATTERN 4: Structure array traversal */
    struct S *struct_ptr = arr_struct;
    for (int i = 0; i < 50; i++) {
        /* Structure field accesses - decompose to base + offset */
        total_sum += struct_ptr->a;      /* Offset 0 for first int field */
        float_sum += struct_ptr->b;      /* Non-zero offset */
        
        /* Post-increment on struct pointer */
        struct_ptr++;
        
        /* Access with array indexing */
        total_sum += arr_struct[i].s;    /* Different offset */
    }
    
    /* PATTERN 5: Mixed types and pointer arithmetic */
    char *c_ptr = arr_c;
    short *s_ptr = arr_s;
    double *d_ptr = arr_d;
    
    for (int i = 0; i < 50; i++) {
        /* Different sized accesses */
        total_sum += *c_ptr++;
        total_sum += *s_ptr++;
        float_sum += *d_ptr++;
        
        /* Compound expressions with pre-decrement */
        if (i > 10) {
            total_sum += arr_s[--i];  /* Pre-decrement in index */
            i++;  /* Restore */
        }
    }
    
    /* PATTERN 6: While loop with post-decrement */
    float *f_ptr = arr_f + 199;
    while (f_ptr >= arr_f + 100) {
        float_sum += *f_ptr--;  /* Post-decrement */
        
        /* Inline assembly to clobber memory */
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Call helper function for additional context */
    total_sum += process_subset(arr_i + 150, 25);
    float_sum += process_floats(arr_f + 125, 25);
    
    /* PATTERN 7: Complex expression with multiple increments */
    int idx = 0;
    int *base1 = arr_i;
    int *base2 = arr_i + 100;
    
    for (int i = 0; i < 25; i++) {
        /* Multiple memory accesses in one expression */
        total_sum += base1[idx] + base2[idx];
        
        /* Increment in same statement as access */
        total_sum += arr_i[idx++];  /* Post-increment in array index */
        
        /* Reset pointer with different base */
        if (i % 5 == 0) {
            base1 = arr_i + (i * 2);
            base2 = arr_i + 100 + (i * 2);
        }
    }
    
    /* Final checksum to prevent elimination */
    printf("Integer checksum: %d\n", total_sum);
    printf("Float checksum: %f\n", float_sum);
    
    return (total_sum > 0) ? 0 : 1;
}
