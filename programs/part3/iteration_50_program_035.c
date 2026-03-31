/* auto-inc-dec-trigger.c */
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
    long l;
} __attribute__((packed));

/* Restrict-qualified helper function */
__attribute__((noinline, optimize("O3")))
static double process_restricted(int* __restrict p1, float* __restrict p2, 
                                 struct S* __restrict p3, int n) {
    double sum = 0.0;
    int i;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (i = 0; i < n; i++) {
        sum += p1[i];  /* [reg + 0] addressing */
        sum += p2[i];  /* Another [reg + 0] */
    }
    
    /* Pattern 2: Mixed pointer/index in same loop */
    struct S* ptr = p3;
    int idx = 0;
    while (idx < n) {
        /* Structure field accesses with constant offsets (some zero) */
        sum += ptr->c;    /* First field, offset 0 */
        sum += ptr->s;
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->d;
        sum += ptr->l;
        
        ptr = &p3[idx];  /* Reset pointer with base + index */
        idx++;
    }
    
    return sum;
}

/* Function with attribute to force specific optimization */
__attribute__((optimize("O3"), noinline))
static void complex_pointer_patterns(int* arr_i, float* arr_f, 
                                     struct S* arr_s, int size) {
    volatile double barrier = 0.0; /* Compiler barrier */
    
    /* Pattern 1: Post-increment in for loop */
    register int* p = arr_i; /* register hint */
    int sum1 = 0;
    
    #pragma GCC unroll 4
    for (int i = 0; i < size; i++) {
        sum1 += *p++;  /* Should trigger auto-inc */
        barrier = sin((double)i); /* Function call between ops */
    }
    
    /* Pattern 2: Post-decrement in while loop */
    float* q = &arr_f[size - 1];
    float sum2 = 0.0f;
    int count = size;
    
    while (count-- > 0) {
        sum2 += *q--;  /* Should trigger auto-dec */
        /* Compound expression with index modification */
        arr_f[count] = sum2 * 0.5f;
    }
    
    /* Pattern 3: Nested loops with pointer reset */
    struct S* r = arr_s;
    double sum3 = 0.0;
    
    for (int outer = 0; outer < 3; outer++) {
        r = arr_s;  /* Reset pointer in outer loop */
        for (int inner = 0; inner < size/2; inner++) {
            /* Mixed structure field accesses */
            sum3 += r->a + r->b;
            r++;  /* Pointer increment */
            
            /* Call obscures aliasing */
            barrier = cos(sum3);
        }
    }
    
    /* Pattern 4: Mixed pointer and index arithmetic */
    int* base = arr_i;
    int sum4 = 0;
    
    for (int i = 0; i < size; i++) {
        /* *(base + i) pattern */
        sum4 += *(base + i);
        
        /* arr_i[i++] in same statement */
        if (i % 2 == 0) {
            sum4 += arr_i[i++];  /* Compound with side effect */
        }
    }
    
    /* Pattern 5: Different pointer aliases to same base */
    int* alias1 = arr_i;
    int* alias2 = arr_i;
    int* alias3 = arr_i;
    
    for (int i = 0; i < size; i += 4) {
        /* Multiple [reg + 0] accesses with different registers */
        sum4 += *alias1; alias1 += 4;
        sum4 += *alias2; alias2 += 4;
        sum4 += *alias3; alias3 += 4;
        
        /* Inline asm clobbers memory */
        __asm__ volatile ("" ::: "memory");
    }
    
    printf("Sums: %d, %.2f, %.2f, %d\n", sum1, sum2, sum3, sum4);
}

int main(void) {
    const int INT_SIZE = 100;
    const int FLOAT_SIZE = 100;
    const int STRUCT_SIZE = 50;
    
    /* Arrays of different types and alignments */
    int arr_i[INT_SIZE] __attribute__((aligned(16)));
    float arr_f[FLOAT_SIZE] __attribute__((aligned(8)));
    struct S arr_s[STRUCT_SIZE];
    
    /* Initialize with varying values */
    srand(42);
    for (int i = 0; i < INT_SIZE; i++) {
        arr_i[i] = rand() % 100;
    }
    
    for (int i = 0; i < FLOAT_SIZE; i++) {
        arr_f[i] = (float)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < STRUCT_SIZE; i++) {
        arr_s[i].c = (char)(rand() % 256);
        arr_s[i].s = (short)(rand() % 1000);
        arr_s[i].a = rand() % 100;
        arr_s[i].b = (float)rand() / RAND_MAX;
        arr_s[i].d = (double)rand() / RAND_MAX;
        arr_s[i].l = rand() * 100L;
    }
    
    /* Execute pattern functions */
    complex_pointer_patterns(arr_i, arr_f, arr_s, INT_SIZE);
    
    /* Call restricted helper */
    double restricted_sum = process_restricted(arr_i, arr_f, arr_s, 25);
    printf("Restricted sum: %.2f\n", restricted_sum);
    
    /* Additional patterns in main */
    
    /* Pattern: Array indexing with pre/post inc/dec */
    int idx = 0;
    int sum5 = 0;
    while (idx < INT_SIZE) {
        sum5 += arr_i[idx++];  /* post-increment in array index */
        sum5 += arr_i[--idx];  /* pre-decrement in array index */
        idx++;
        
        /* Function call between memory ops */
        sin((double)sum5);
    }
    
    /* Pattern: Pointer arithmetic with different strides */
    char* char_ptr = (char*)arr_i;
    short* short_ptr = (short*)arr_i;
    int* int_ptr = arr_i;
    
    for (int i = 0; i < 20; i++) {
        *char_ptr++ = (char)i;
        *short_ptr++ = (short)i;
        *int_ptr++ = i;
        
        /* Memory clobber */
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Final checksum to prevent elimination */
    long checksum = 0;
    for (int i = 0; i < INT_SIZE; i++) {
        checksum += arr_i[i];
    }
    for (int i = 0; i < FLOAT_SIZE; i++) {
        checksum += (long)arr_f[i];
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}

#pragma GCC pop_options
