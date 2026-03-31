/* test_auto_inc_dec.c
 * Designed to trigger the uncovered lines in GCC's auto-inc-dec pass.
 * The pattern should create memory accesses with base+0 addressing
 * and separate increment instructions that find_inc(true) can merge.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
void pattern1_simple_pointer(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    
    /* Classic *ptr++ pattern - access with offset 0, then increment */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Should become *(p + 0) after optimization */
        p++;        /* Separate increment instruction */
    }
    
    printf("Pattern1 sum: %d\n", sum);
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void pattern2_indexed_access(char *buffer, int size) {
    /* Simple memset-like pattern */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(base + 0) after ivopts */
    }
    
    /* Verify */
    for (int i = 0; i < size; i++) {
        if (buffer[i] != 0) {
            printf("Pattern2 failed at %d\n", i);
        }
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void pattern3_nested_loops(int matrix[M][N]) {
    /* Outer loop calculates base, inner loop uses it */
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base calculated in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
    
    /* Verify */
    int total = 0;
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            total += matrix[j][i];
        }
    }
    printf("Pattern3 total: %d\n", total);
}

/* Pattern 4: Explicit stride with separate increment */
void pattern4_explicit_stride(float *data, int count, int stride) {
    float total = 0.0f;
    float *ptr = data;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < count; i++) {
        total += *ptr;  /* *(ptr + 0) */
        ptr += stride;  /* Separate increment by constant stride */
    }
    
    printf("Pattern4 total: %f\n", total);
}

/* Pattern 5: Struct access with pointer increment */
struct element {
    int key;
    float value;
    char tag;
};

void pattern5_struct_access(struct element *arr, int size) {
    int sum_keys = 0;
    struct element *ptr = arr;
    
    /* Access struct through pointer, then increment */
    for (int i = 0; i < size; i++) {
        sum_keys += ptr->key;  /* Should be *(ptr + 0).key */
        ptr++;                 /* Increment by sizeof(struct element) */
    }
    
    printf("Pattern5 sum_keys: %d\n", sum_keys);
}

/* Pattern 6: Multiple memory accesses with same base */
void pattern6_multiple_accesses(int *src, int *dst, int size) {
    int *s = src;
    int *d = dst;
    
    /* Two memory accesses with the same base+0 pattern */
    for (int i = 0; i < size; i++) {
        int val = *s;  /* First access: *(s + 0) */
        *d = val;      /* Second access: *(d + 0) */
        s++;
        d++;
    }
    
    /* Verify copy */
    for (int i = 0; i < size; i++) {
        if (src[i] != dst[i]) {
            printf("Pattern6 mismatch at %d\n", i);
        }
    }
}

/* Pattern 7: Loop with if condition that doesn't prevent optimization */
void pattern7_conditional_access(int *data, int size, int threshold) {
    int count = 0;
    int *ptr = data;
    
    /* The increment is still separate from the access */
    for (int i = 0; i < size; i++) {
        if (*ptr > threshold) {  /* *(ptr + 0) */
            count++;
        }
        ptr++;  /* Increment happens regardless of condition */
    }
    
    printf("Pattern7 count: %d\n", count);
}

int main() {
    /* Initialize test data */
    int arr[N];
    char buffer[N];
    int matrix[M][N];
    float farr[N * 2];
    struct element struct_arr[N];
    int src[N], dst[N];
    int data[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = 'a' + (i % 26);
        src[i] = i * 2;
        data[i] = i % 10;
    }
    
    for (int i = 0; i < N * 2; i++) {
        farr[i] = i * 0.5f;
    }
    
    for (int i = 0; i < N; i++) {
        struct_arr[i].key = i;
        struct_arr[i].value = i * 1.5f;
        struct_arr[i].tag = 'A' + (i % 26);
    }
    
    /* Execute all patterns */
    pattern1_simple_pointer(arr, N);
    pattern2_indexed_access(buffer, N);
    pattern3_nested_loops(matrix);
    pattern4_explicit_stride(farr, N, 2);
    pattern5_struct_access(struct_arr, N);
    pattern6_multiple_accesses(src, dst, N);
    pattern7_conditional_access(data, N, 5);
    
    return 0;
}
