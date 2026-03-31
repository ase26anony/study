/* test_auto_inc_dec.c
 * 
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns create memory accesses with base+0 addressing where
 * find_inc(true) can find a preceding increment instruction.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment */
int pattern1_sum_array(const int* arr, int size) {
    int sum = 0;
    const int* p = arr;
    
    /* Classic *ptr++ pattern - access uses *(ptr + 0), then ptr is incremented */
    for (int i = 0; i < size; i++) {
        sum += *p;      /* Should become base + 0 access */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access that gets transformed to pointer */
void pattern2_zero_buffer(int* buffer, int size) {
    /* Simple indexed access - ivopts may create pointer induction variable */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(base + 0) with separate increment */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void pattern3_matrix_init(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int* base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + offset */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* May become *(base + 0) in RTL */
        }
    }
}

/* Pattern 4: Explicit pointer arithmetic with stride */
int pattern4_stride_access(const int* arr, int size, int stride) {
    int total = 0;
    const int* ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < size; i++) {
        total += *ptr;      /* *(ptr + 0) */
        ptr += stride;      /* Explicit increment by constant */
    }
    return total;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int a;
    int b;
    float c;
    char d[4];
};

int pattern5_struct_sum(struct Data* array, int size) {
    int sum = 0;
    struct Data* ptr = array;
    
    /* Struct access - larger stride may still trigger optimization */
    for (int i = 0; i < size; i++) {
        sum += ptr->a;      /* Access at offset 0 within struct */
        ptr++;              /* Increment by sizeof(struct Data) */
    }
    return sum;
}

/* Pattern 6: Mixed patterns to increase coverage */
void pattern6_mixed(int* dest, const int* src1, const int* src2, int size) {
    const int* p1 = src1;
    const int* p2 = src2;
    int* p3 = dest;
    
    /* Multiple pointers with independent increments */
    for (int i = 0; i < size; i++) {
        *p3 = *p1 + *p2;   /* Three memory accesses, all base + 0 */
        p1++;
        p2++;
        p3++;
    }
}

/* Pattern 7: Loop with if condition that doesn't break the pattern */
int pattern7_conditional_sum(const int* arr, int size, int threshold) {
    int sum = 0;
    const int* p = arr;
    
    for (int i = 0; i < size; i++) {
        if (*p > threshold) {  /* Access with base + 0 */
            sum += *p;          /* Another access with base + 0 */
        }
        p++;                    /* Increment after condition */
    }
    return sum;
}

/* Pattern 8: Do-while loop variant */
int pattern8_dowhile_sum(const int* arr, int size) {
    int sum = 0;
    const int* p = arr;
    int count = size;
    
    do {
        sum += *p;      /* base + 0 access */
        p++;            /* increment */
    } while (--count > 0);
    
    return sum;
}

/* Main function to exercise all patterns */
int main() {
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    struct Data struct_arr[N];
    int dest[N];
    int src1[N], src2[N];
    
    /* Initialize test data */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = 0;
        src1[i] = i * 2;
        src2[i] = i * 3;
        struct_arr[i].a = i;
        struct_arr[i].b = i * 2;
        struct_arr[i].c = i * 1.5f;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = 0;
        }
    }
    
    /* Execute all patterns */
    int sum1 = pattern1_sum_array(arr, N);
    printf("Pattern 1 sum: %d\n", sum1);
    
    pattern2_zero_buffer(buffer, N);
    printf("Pattern 2 buffer[0]: %d\n", buffer[0]);
    
    pattern3_matrix_init(matrix);
    printf("Pattern 3 matrix[0][0]: %d\n", matrix[0][0]);
    
    int sum4 = pattern4_stride_access(arr, N, 1);
    printf("Pattern 4 sum: %d\n", sum4);
    
    int sum5 = pattern5_struct_sum(struct_arr, N);
    printf("Pattern 5 sum: %d\n", sum5);
    
    pattern6_mixed(dest, src1, src2, N);
    printf("Pattern 6 dest[0]: %d\n", dest[0]);
    
    int sum7 = pattern7_conditional_sum(arr, N, N/2);
    printf("Pattern 7 sum: %d\n", sum7);
    
    int sum8 = pattern8_dowhile_sum(arr, N);
    printf("Pattern 8 sum: %d\n", sum8);
    
    return 0;
}
