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
    long l;
} __attribute__((packed));

/* Restrict-qualified helper function */
__attribute__((noinline, optimize("O3")))
static double process_restricted(int* __restrict p1, float* __restrict p2, 
                                 struct S* __restrict p3, int n) {
    double sum = 0.0;
    
    /* Pattern 1: Mixed pointer arithmetic with zero offset */
    for (int i = 0; i < n; i++) {
        sum += *p1;      /* [reg + 0] addressing */
        sum += *p2;      /* Different base register, zero offset */
        p1++;
        p2++;
    }
    
    /* Pattern 2: Structure field access with constant offsets */
    for (int i = 0; i < n; i++) {
        sum += p3->a;    /* Base + 0 offset (after packed char/short) */
        sum += p3->b;    /* Base + constant offset */
        p3++;
    }
    
    return sum;
}

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O3"), noinline))
static void mixed_access_patterns(int* arr_i, float* arr_f, 
                                  struct S* arr_s, int n) {
    register int* p1 asm ("r12") = arr_i;
    register float* p2 asm ("r13") = arr_f;
    double checksum = 0.0;
    
    /* Pattern 1: Post-increment in loop condition */
    int count = n;
    while (count-- > 0) {
        checksum += *p1++;  /* Should trigger find_inc for [reg+0] */
        
        /* Insert function call to force re-evaluation */
        if (count % 16 == 0) {
            checksum += sin(checksum);
        }
    }
    
    /* Pattern 2: Post-decrement with reset */
    p1 = arr_i + n - 1;  /* Reset pointer */
    for (int i = n; i > 0; ) {
        checksum += *p1--;  /* Post-decrement */
        i--;
        
        /* Multiple base registers with zero offset */
        checksum += arr_f[i];  /* [base + 0] when i=0 in next iteration */
    }
    
    /* Pattern 3: Nested loops with pointer switching */
    for (int outer = 0; outer < 3; outer++) {
        struct S* ptr = arr_s;
        for (int inner = 0; inner < n/2; inner++) {
            /* Structure field accesses - decompose to base + constant */
            checksum += ptr->a;  /* First int field after packed members */
            checksum += ptr->b;
            ptr++;  /* Auto-inc opportunity */
        }
        
        /* Compiler barrier via function call */
        checksum = cos(checksum * 0.01);
    }
    
    /* Pattern 4: Mixed index and pointer arithmetic */
    for (int idx = 0; idx < n; idx++) {
        /* Create [reg + 0] pattern through pointer arithmetic */
        int* temp_ptr = arr_i + idx;
        checksum += *temp_ptr;  /* [reg + 0] addressing */
        
        /* Compound expression with index modification */
        checksum += arr_f[idx++];  /* arr[--index] pattern when reversed */
        idx--;  /* Reset for loop increment */
    }
    
    printf("Checksum: %f\n", checksum);
}

int main(void) {
    const int INT_SIZE = 100;
    const int FLOAT_SIZE = 100;
    const int STRUCT_SIZE = 50;
    
    /* Declare arrays with different types and alignments */
    int arr_i[INT_SIZE] __attribute__((aligned(16)));
    float arr_f[FLOAT_SIZE] __attribute__((aligned(8)));
    struct S arr_s[STRUCT_SIZE];
    
    /* Initialize with varying values */
    for (int i = 0; i < INT_SIZE; i++) {
        arr_i[i] = rand() % 1000;
    }
    
    for (int i = 0; i < FLOAT_SIZE; i++) {
        arr_f[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    for (int i = 0; i < STRUCT_SIZE; i++) {
        arr_s[i].c = rand() % 256;
        arr_s[i].s = rand() % 1000;
        arr_s[i].a = rand() % 10000;
        arr_s[i].b = (float)rand() / RAND_MAX;
        arr_s[i].d = (double)rand() / RAND_MAX * 1000.0;
        arr_s[i].l = rand() * 1000L;
    }
    
    /* Pattern A: Simple post-increment loop */
    double total = 0.0;
    {
        int* p = arr_i;
        #pragma GCC unroll 4
        for (int i = 0; i < INT_SIZE; i++) {
            total += *p++;  /* Classic auto-inc pattern */
        }
    }
    
    /* Pattern B: Post-decrement while loop with multiple bases */
    {
        float* q = arr_f + FLOAT_SIZE - 1;
        int counter = FLOAT_SIZE;
        while (counter) {
            total += *q--;  /* Post-decrement */
            counter--;
            
            /* Access with zero offset through different base */
            if (counter == 0) {
                total += arr_i[0];  /* [base + 0] */
            }
        }
    }
    
    /* Pattern C: Nested loops with structure access */
    {
        struct S* ptr = arr_s;
        for (int outer = 0; outer < 2; outer++) {
            for (int inner = 0; inner < STRUCT_SIZE; inner++) {
                /* Multiple field accesses from same base */
                total += ptr->a;  /* Base + constant offset */
                total += ptr->b;
                ptr++;  /* Pointer increment - auto-inc opportunity */
            }
            ptr = arr_s;  /* Reset for next outer iteration */
            
            /* Function call between pointer operations */
            total = fabs(sin(total * 0.001));
        }
    }
    
    /* Pattern D: Mixed pointer/index arithmetic */
    {
        for (int i = 0; i < INT_SIZE; i++) {
            /* Pointer arithmetic creating [reg + 0] pattern */
            int* alias_ptr = &arr_i[i];
            total += *alias_ptr;  /* [reg + 0] */
            
            /* Compound index expression */
            total += arr_f[INT_SIZE - i - 1];  /* arr[--index] pattern */
        }
    }
    
    /* Call function with restrict qualifiers */
    total += process_restricted(arr_i, arr_f, arr_s, 25);
    
    /* Call function with mixed patterns */
    mixed_access_patterns(arr_i, arr_f, arr_s, 50);
    
    /* Final checksum to prevent dead code elimination */
    volatile double final_result = total;
    printf("Final result: %f\n", final_result);
    
    return 0;
}
