/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#pragma GCC optimize("O3")
#pragma GCC push_options

/* Heterogeneous structure with mixed alignment */
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
        sum += p1[i];  /* [reg + 0] addressing for first element */
        sum += p2[i];
    }
    
    /* Pattern 2: Pointer arithmetic with constant zero offset */
    struct S* ptr = p3;
    for (i = 0; i < n; i++) {
        sum += ptr->a;  /* Base + 0 offset for first field */
        sum += ptr->b;
        ptr++;
    }
    
    return sum;
}

/* Function with attribute to force specific optimization */
__attribute__((optimize("O3"), noinline))
static void mixed_access_patterns(int* arr_i, float* arr_f, 
                                  struct S* arr_s, int size) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    /* Pattern 1: Post-increment in for loop */
    register int* p = arr_i;  /* register hint */
    for (int i = 0; i < size; i++) {
        sum_int += *p++;  /* Should trigger find_inc for post-increment */
        
        /* Function call to obscure aliasing */
        if (i % 16 == 0) {
            sum_float += sinf((float)sum_int);
        }
    }
    
    /* Pattern 2: Post-decrement in while loop */
    float* q = &arr_f[size - 1];
    int count = size;
    while (count-- > 0) {
        sum_float += *q--;  /* Should trigger find_inc for post-decrement */
        
        /* Inline assembly as compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Pattern 3: Nested loops with pointer reset */
    for (int outer = 0; outer < 3; outer++) {
        struct S* sptr = arr_s;
        for (int inner = 0; inner < size / 2; inner++) {
            /* Multiple field accesses with same base */
            sum_int += sptr->a;    /* Base + constant offset (often 0) */
            sum_double += sptr->d; /* Different offset */
            sptr++;  /* Pointer increment */
        }
        
        /* Function call between pointer operations */
        sum_float = cosf(sum_float);
    }
    
    /* Pattern 4: Mixed pointer and index arithmetic */
    int* base = arr_i;
    for (int idx = 0; idx < size; idx++) {
        /* Compound expression with index modification */
        sum_int += arr_i[idx++];  /* arr[index++] pattern */
        if (idx < size) {
            sum_int += *(base + idx);  /* *(ptr + i) pattern */
        }
        
        /* Another function call barrier */
        sum_double += sqrt(fabs((double)sum_int));
    }
    
    /* Pattern 5: Different pointer aliases to same memory */
    int* alias1 = arr_i;
    int* alias2 = arr_i;
    float* alias3 = arr_f;
    
    #pragma GCC unroll 4
    for (int i = 0; i < size; i += 2) {
        /* Multiple accesses with [reg + 0] addressing */
        sum_int += alias1[i];     /* First alias */
        sum_int += alias2[i + 1]; /* Second alias, different offset */
        sum_float += alias3[i];   /* Different type, same base concept */
    }
    
    /* Prevent dead code elimination */
    volatile int sink __attribute__((unused)) = sum_int;
    volatile float fsink __attribute__((unused)) = sum_float;
    volatile double dsink __attribute__((unused)) = sum_double;
}

int main() {
    const int INT_SIZE = 100;
    const int FLOAT_SIZE = 100;
    const int STRUCT_SIZE = 50;
    
    /* Arrays of different types and alignments */
    int arr_i[INT_SIZE] __attribute__((aligned(16)));
    float arr_f[FLOAT_SIZE] __attribute__((aligned(8)));
    struct S arr_s[STRUCT_SIZE];
    
    /* Initialize with random but reproducible values */
    srand(42);
    for (int i = 0; i < INT_SIZE; i++) {
        arr_i[i] = rand() % 1000;
    }
    
    for (int i = 0; i < FLOAT_SIZE; i++) {
        arr_f[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < STRUCT_SIZE; i++) {
        arr_s[i].c = (char)(rand() % 256);
        arr_s[i].s = (short)(rand() % 1000);
        arr_s[i].a = rand() % 1000;
        arr_s[i].b = (float)(rand() % 1000) / 10.0f;
        arr_s[i].d = (double)(rand() % 1000) / 10.0;
        arr_s[i].l = rand() % 1000;
    }
    
    /* Call pattern functions multiple times */
    double total_sum = 0.0;
    
    for (int iteration = 0; iteration < 5; iteration++) {
        /* Function with mixed patterns */
        mixed_access_patterns(arr_i, arr_f, arr_s, INT_SIZE);
        
        /* Restricted pointer function */
        total_sum += process_restricted(arr_i, arr_f, arr_s, 
                                       INT_SIZE < STRUCT_SIZE ? INT_SIZE : STRUCT_SIZE);
        
        /* Additional loop with decrement pattern */
        int* p = &arr_i[INT_SIZE - 1];
        int local_sum = 0;
        for (int i = INT_SIZE - 1; i >= 0; i--) {
            local_sum += *p--;  /* Post-decrement */
            
            /* Function call every 8 iterations */
            if (i % 8 == 0) {
                total_sum += sin((double)local_sum);
            }
        }
        
        /* Array indexing with pre-decrement */
        int index = INT_SIZE - 1;
        while (index > 0) {
            local_sum += arr_i[--index];  /* --index in array access */
            
            /* Memory clobber */
            asm volatile("" ::: "memory");
        }
    }
    
    /* Final checksum to prevent elimination */
    printf("Checksum: %f\n", total_sum);
    
    return 0;
}

#pragma GCC pop_options
