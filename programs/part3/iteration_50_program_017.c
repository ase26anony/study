/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")
#pragma GCC push_options

/* Heterogeneous structure with different alignments */
struct S {
    char c;
    short s;
    int a;
    float b;
    double d;
    long long ll;
} __attribute__((packed));

/* Restrict-qualified helper function */
__attribute__((noinline, optimize("O3")))
static double process_restricted(int* __restrict p1, float* __restrict p2, 
                                 struct S* __restrict p3, int n) {
    double sum = 0.0;
    
    /* Pattern 1: Mixed pointer arithmetic with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *(p1 + 0);  /* reg + 0 addressing */
        sum += *(p2 + 0);  /* another reg + 0 */
        p1++;
        p2++;
    }
    
    /* Pattern 2: Structure field access (base + constant offset, often 0) */
    for (int i = 0; i < n; i++) {
        sum += p3->a;  /* First field often has offset 0 */
        sum += p3->b;
        p3++;
    }
    
    return sum;
}

/* Function with attribute to force specific optimization */
__attribute__((optimize("O3"), noinline))
static void mixed_access_patterns(void) {
    /* Declare arrays with different types and alignments */
    int arr_i[200];
    float arr_f[200];
    double arr_d[200];
    short arr_s[200];
    struct S arr_struct[100];
    
    /* Initialize with varying values */
    for (int i = 0; i < 200; i++) {
        arr_i[i] = rand() % 100;
        arr_f[i] = (float)(rand() % 100) / 10.0f;
        arr_d[i] = (double)(rand() % 100) / 5.0;
        arr_s[i] = (short)(rand() % 100);
    }
    
    for (int i = 0; i < 100; i++) {
        arr_struct[i].c = (char)(rand() % 256);
        arr_struct[i].s = (short)(rand() % 100);
        arr_struct[i].a = rand() % 100;
        arr_struct[i].b = (float)(rand() % 100) / 10.0f;
        arr_struct[i].d = (double)(rand() % 100) / 5.0;
        arr_struct[i].ll = (long long)rand() * rand();
    }
    
    volatile double checksum = 0.0;
    
    /* PATTERN 1: Simple post-increment in for loop */
    #pragma GCC unroll 4
    for (int* p = arr_i; p < &arr_i[200]; ) {
        checksum += *p++;  /* Should trigger auto-inc */
    }
    
    /* Compiler barrier via math function */
    checksum += sin(checksum);
    
    /* PATTERN 2: Post-decrement in while loop */
    float* q = &arr_f[199];
    while (q >= arr_f) {
        checksum += *q--;  /* Should trigger auto-dec */
    }
    
    /* PATTERN 3: Nested loops with pointer reset */
    for (int outer = 0; outer < 10; outer++) {
        double* r = arr_d;
        for (int inner = 0; inner < 20; inner++) {
            checksum += *r++;  /* Inner loop pointer increment */
        }
        /* Outer loop resets pointer - creates new context for find_inc */
    }
    
    /* PATTERN 4: Mixed index and pointer arithmetic */
    register short* base_ptr = arr_s;  /* register hint */
    for (int idx = 0; idx < 200; idx++) {
        checksum += *(base_ptr + idx);  /* reg + idx addressing */
    }
    
    /* Compiler barrier */
    checksum += cos(checksum);
    
    /* PATTERN 5: Array indexing with increment in expression */
    int index = 0;
    while (index < 200) {
        checksum += arr_i[index++];  /* arr[index++] pattern */
        checksum += arr_f[--index];  /* arr[--index] pattern */
        index++;
    }
    
    /* PATTERN 6: Structure array traversal */
    struct S* struct_ptr = arr_struct;
    for (int i = 0; i < 100; i++) {
        /* Multiple field accesses from same base */
        checksum += struct_ptr->a;  /* Base + 0 offset for first int field */
        checksum += struct_ptr->b;
        checksum += struct_ptr->d;
        struct_ptr++;  /* Pointer increment */
    }
    
    /* PATTERN 7: Multiple pointer aliases with zero offset */
    int* p1 = arr_i;
    int* p2 = arr_i + 50;
    int* p3 = arr_i + 100;
    
    for (int i = 0; i < 50; i++) {
        checksum += *(p1 + 0);  /* reg + 0 */
        checksum += *(p2 + 0);  /* different reg + 0 */
        checksum += *(p3 + 0);  /* another reg + 0 */
        p1++;
        p2++;
        p3++;
    }
    
    /* PATTERN 8: Inline assembly as compiler barrier */
    asm volatile("" : : : "memory");
    
    /* PATTERN 9: Call to function with restrict qualifiers */
    checksum += process_restricted(arr_i, arr_f, arr_struct, 50);
    
    /* Prevent dead code elimination */
    volatile double* dummy = (volatile double*)malloc(sizeof(double));
    *dummy = checksum;
    printf("Checksum: %f\n", *dummy);
    free((void*)dummy);
}

int main(void) {
    /* Seed RNG for reproducible results */
    srand(42);
    
    /* Call the function with various patterns */
    mixed_access_patterns();
    
    /* Additional context with different optimization level */
    #pragma GCC push_options
    #pragma GCC optimize("O1")
    {
        /* Simple loop that might be optimized differently */
        int local_arr[100];
        for (int i = 0; i < 100; i++) {
            local_arr[i] = i;
        }
        
        int sum = 0;
        int* ptr = local_arr;
        for (int i = 0; i < 100; i++) {
            sum += *ptr++;
        }
        
        printf("Secondary sum: %d\n", sum);
    }
    #pragma GCC pop_options
    
    return 0;
}
