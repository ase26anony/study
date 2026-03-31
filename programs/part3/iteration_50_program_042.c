/* Compile with: gcc -O2 -fno-inline -fno-strict-aliasing -fdump-rtl-auto_inc_dec -o test_auto_inc test_auto_inc.c -lm */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 100
#define STRUCT_SIZE 50

/* Mixed data type struct */
struct S {
    int a;
    float b;
    double c;
    char d;
    short e;
    long f;
};

/* __attribute__((noinline)) helper function */
__attribute__((noinline)) 
static void process_subset(int* restrict arr_int, float* restrict arr_float, 
                          struct S* restrict arr_struct, int n) {
    register int* p = arr_int;
    register float* q = arr_float;
    register struct S* s = arr_struct;
    
    /* Pattern 1: Multiple base registers with zero offset */
    for (int i = 0; i < n; i++) {
        /* These should trigger [reg + 0] addressing */
        *p = *p * 2;
        *q = *q * 1.5f;
        s->a = s->a + 1;  /* First field has offset 0 */
        p++;
        q++;
        s++;
    }
}

/* Function with aggressive optimization */
__attribute__((optimize("O3")))
static void optimized_loop_patterns(int* arr_int, float* arr_float, 
                                   double* arr_double, struct S* arr_struct) {
    /* Pattern 1: Post-increment in for loop */
    int sum1 = 0;
    register int* p = arr_int;
    for (int i = 0; i < SIZE; i++) {
        sum1 += *p++;  /* Should trigger auto-inc */
    }
    
    /* Compiler barrier with function call */
    double barrier = sin((double)sum1);
    
    /* Pattern 2: Post-decrement in while loop */
    float sum2 = 0.0f;
    float* q = arr_float + SIZE - 1;
    int count = SIZE;
    while (count-- > 0) {
        sum2 += *q--;  /* Should trigger auto-dec */
    }
    
    /* Pattern 3: Mixed pointer and index with zero offset */
    double sum3 = 0.0;
    #pragma GCC unroll 4
    for (int i = 0; i < SIZE; i++) {
        /* This should create [reg + 0] pattern */
        sum3 += *(arr_double + i);
    }
    
    /* Pattern 4: Nested loops with pointer reset */
    long sum4 = 0;
    for (int outer = 0; outer < 5; outer++) {
        register struct S* s = arr_struct;
        for (int inner = 0; inner < STRUCT_SIZE; inner++) {
            /* Access different struct fields with constant offsets */
            sum4 += s->a;      /* offset 0 */
            sum4 += (long)(s->b * 100.0f);  /* offset 4 */
            sum4 += (long)s->c;  /* offset 8 or 16 depending on alignment */
            s++;  /* Pointer increment */
        }
        /* Function call between loops */
        barrier += cos((double)outer);
    }
    
    /* Pattern 5: Compound expressions in loop conditions */
    int idx = 0;
    int sum5 = 0;
    while (idx < SIZE) {
        /* arr_int[idx++] creates index modification with access */
        sum5 += arr_int[idx++];
        /* Insert inline assembly as compiler barrier */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Pattern 6: Pre-decrement in array access */
    idx = SIZE;
    int sum6 = 0;
    while (idx > 0) {
        sum6 += arr_int[--idx];
    }
    
    /* Use results to prevent dead code elimination */
    arr_int[0] = (int)(sum1 + sum5 + sum6);
    arr_float[0] = (float)(sum2 + barrier);
    arr_double[0] = sum3;
}

int main() {
    /* Declare arrays of different types and alignments */
    int __attribute__((aligned(16))) arr_int[SIZE];
    float arr_float[SIZE];
    double __attribute__((aligned(32))) arr_double[SIZE];
    struct S arr_struct[STRUCT_SIZE];
    
    /* Initialize with random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = rand() % 1000;
        arr_float[i] = (float)(rand() % 1000) / 10.0f;
        arr_double[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < STRUCT_SIZE; i++) {
        arr_struct[i].a = rand() % 1000;
        arr_struct[i].b = (float)(rand() % 1000) / 10.0f;
        arr_struct[i].c = (double)(rand() % 1000) / 10.0;
        arr_struct[i].d = (char)(rand() % 256);
        arr_struct[i].e = (short)(rand() % 1000);
        arr_struct[i].f = (long)(rand() % 1000);
    }
    
    /* Call optimized function */
    optimized_loop_patterns(arr_int, arr_float, arr_double, arr_struct);
    
    /* Call noinline helper function */
    process_subset(arr_int + 10, arr_float + 10, arr_struct + 5, 20);
    
    /* Additional pattern: Multiple pointer aliases with zero offset */
    int* restrict p1 = arr_int;
    int* restrict p2 = arr_int + SIZE/2;
    float* restrict f1 = arr_float;
    float* restrict f2 = arr_float + SIZE/2;
    
    int checksum = 0;
    for (int i = 0; i < SIZE/4; i++) {
        /* Multiple [reg + 0] accesses */
        *p1 = *p1 * 3;
        *p2 = *p2 * 2;
        *f1 = *f1 * 1.1f;
        *f2 = *f2 * 0.9f;
        
        checksum += *p1 + *p2 + (int)*f1 + (int)*f2;
        
        p1++;
        p2++;
        f1++;
        f2++;
        
        /* Function call to obscure aliasing */
        if (i % 8 == 0) {
            double temp = sqrt((double)checksum);
            checksum += (int)temp;
        }
    }
    
    /* Pattern: Heterogeneous access in single loop */
    char* byte_ptr = (char*)arr_int;
    short* short_ptr = (short*)arr_int;
    int* int_ptr = arr_int;
    
    for (int i = 0; i < SIZE/2; i++) {
        /* Different sized accesses from same base */
        *byte_ptr = (*byte_ptr) ^ 0x55;
        *short_ptr = (*short_ptr) + 1;
        *int_ptr = (*int_ptr) - 1;
        
        byte_ptr += sizeof(int);
        short_ptr += 2;  /* Step by 2 shorts to match int alignment */
        int_ptr++;
    }
    
    /* Final checksum computation and output */
    long final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += arr_int[i] + (long)arr_float[i] + (long)arr_double[i];
    }
    
    for (int i = 0; i < STRUCT_SIZE; i++) {
        final_sum += arr_struct[i].a + (long)arr_struct[i].b + 
                    (long)arr_struct[i].c + arr_struct[i].d + 
                    arr_struct[i].e + arr_struct[i].f;
    }
    
    printf("Checksum: %ld\n", final_sum);
    printf("Result: %d\n", checksum);
    
    return 0;
}
