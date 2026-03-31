/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns create memory accesses with base+0 addressing where
 * find_inc(true) can find a preceding increment instruction.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int pattern1_sum_array(const int *arr, int size) {
    int sum = 0;
    const int *ptr = arr;
    
    /* Classic *ptr++ pattern - access uses ptr+0, then ptr is incremented */
    for (int i = 0; i < size; i++) {
        sum += *ptr;  /* Should become *(ptr + 0) after optimization */
        ptr++;        /* Separate increment instruction for find_inc to find */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer induction variable */
void pattern2_clear_buffer(char *buffer, int size) {
    /* Simple indexed access - ivopts may create a pointer induction var */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) with ptr incremented elsewhere */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void pattern3_fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + offset */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit pointer arithmetic with stride */
float pattern4_dot_product(const float *a, const float *b, int size) {
    float sum = 0.0f;
    const float *pa = a;
    const float *pb = b;
    
    /* Explicit separate access and increment */
    for (int i = 0; i < size; i++) {
        sum += (*pa) * (*pb);  /* Two *(ptr + 0) accesses */
        pa += 1;               /* Separate increment instructions */
        pb += 1;
    }
    return sum;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int values[4];
    float weight;
};

int pattern5_sum_struct_values(const struct Data *data, int count) {
    int total = 0;
    const struct Data *ptr = data;
    
    /* Struct access - larger stride may still trigger the pattern */
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 4; j++) {
            total += ptr->values[j];  /* Multiple *(base + const_offset) */
        }
        ptr++;  /* Increment by sizeof(struct Data) */
    }
    return total;
}

/* Pattern 6: While loop with pointer comparison */
void pattern6_copy_reverse(const int *src, int *dst, int size) {
    const int *s = src + size - 1;
    int *d = dst;
    
    /* While loop with decrement */
    while (s >= src) {
        *d = *s;  /* *(ptr + 0) access */
        d++;
        s--;      /* Auto-decrement opportunity */
    }
}

/* Pattern 7: Mixed patterns to increase coverage */
void pattern7_multiple_accesses(int *arr1, int *arr2, int size) {
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < size; i++) {
        /* Multiple memory ops with same base */
        int val = *p1;          /* First access: *(p1 + 0) */
        *p2 = val * 2;          /* Second access: *(p2 + 0) */
        
        /* Some computation between accesses */
        if (val > 100) {
            *p1 = 100;          /* Third access: *(p1 + 0) again */
        }
        
        p1++;
        p2++;
    }
}

int main() {
    /* Initialize test data */
    int array[N];
    char buffer[N];
    int matrix[M][N];
    float vec1[N], vec2[N];
    struct Data structs[N/4];
    
    for (int i = 0; i < N; i++) {
        array[i] = i;
        buffer[i] = (char)i;
        vec1[i] = i * 0.5f;
        vec2[i] = i * 0.25f;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = i + j * N;
        }
    }
    
    for (int i = 0; i < N/4; i++) {
        for (int j = 0; j < 4; j++) {
            structs[i].values[j] = i * 4 + j;
        }
        structs[i].weight = i * 0.1f;
    }
    
    /* Execute all patterns */
    int sum1 = pattern1_sum_array(array, N);
    printf("Pattern 1 sum: %d\n", sum1);
    
    pattern2_clear_buffer(buffer, N);
    printf("Pattern 2 completed\n");
    
    pattern3_fill_matrix(matrix);
    printf("Pattern 3 completed\n");
    
    float dot = pattern4_dot_product(vec1, vec2, N);
    printf("Pattern 4 dot product: %f\n", dot);
    
    int struct_sum = pattern5_sum_struct_values(structs, N/4);
    printf("Pattern 5 struct sum: %d\n", struct_sum);
    
    int reversed[N];
    pattern6_copy_reverse(array, reversed, N);
    printf("Pattern 6 completed\n");
    
    int arr2[N];
    pattern7_multiple_accesses(array, arr2, N);
    printf("Pattern 7 completed\n");
    
    return 0;
}
