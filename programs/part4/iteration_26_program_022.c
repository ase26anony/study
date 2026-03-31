/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto-inc-dec pass where memory addresses have a constant
 * offset of zero (reg1_val = 0) and find_inc(true) is called.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int pattern1_simple_pointer_postinc(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    
    /* Classic *ptr++ pattern - access at ptr+0, then increment */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Should become *(p + 0) after optimization */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void pattern2_indexed_access_postinc(int *buffer, int size) {
    /* The compiler's ivopts pass may transform this into a pointer
     * induction variable with zero offset in the loop body */
    for (int i = 0; i < size; i++) {
        buffer[i] = i;  /* May become *(ptr + 0) where ptr = buffer + i */
    }
}

/* Pattern 3: Explicit pointer increment with stride */
int pattern3_explicit_increment(int *arr, int size, int stride) {
    int total = 0;
    int *ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < size; i++) {
        total += *ptr;  /* *(ptr + 0) */
        ptr += stride;  /* Candidate increment instruction */
    }
    return total;
}

/* Pattern 4: Nested loops with invariant base in inner loop */
void pattern4_nested_invariant_base(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 offset */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* May become *(base + 0) in RTL */
        }
    }
}

/* Pattern 5: Different data types to test various alignments */
float pattern5_float_traversal(float *arr, int size) {
    float sum = 0.0f;
    float *p = arr;
    
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Float access with ptr+0 */
        p++;        /* Increment by sizeof(float) */
    }
    return sum;
}

/* Pattern 6: Struct access to test with non-primitive types */
struct Point {
    int x;
    int y;
    int z;
};

int pattern6_struct_traversal(struct Point *points, int size) {
    int total = 0;
    struct Point *p = points;
    
    for (int i = 0; i < size; i++) {
        total += p->x;  /* Access at p+0 */
        p++;            /* Increment by sizeof(struct Point) */
    }
    return total;
}

/* Pattern 7: Char buffer processing - smallest increment */
int pattern7_char_traversal(char *buffer, int size) {
    int checksum = 0;
    char *p = buffer;
    
    for (int i = 0; i < size; i++) {
        checksum += *p;  /* Char access at p+0 */
        p++;             /* Increment by 1 */
    }
    return checksum;
}

/* Pattern 8: Loop with multiple memory references */
void pattern8_multiple_refs(int *src, int *dst, int size) {
    int *s = src;
    int *d = dst;
    
    for (int i = 0; i < size; i++) {
        *d = *s;  /* Both *(s + 0) and *(d + 0) */
        s++;
        d++;
    }
}

/* Pattern 9: While loop version */
int pattern9_while_loop(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    int count = size;
    
    while (count-- > 0) {
        sum += *p;  /* *(p + 0) */
        p++;        /* Separate increment */
    }
    return sum;
}

/* Pattern 10: Reverse traversal (auto-decrement opportunity) */
int pattern10_reverse_traversal(int *arr, int size) {
    int sum = 0;
    int *p = arr + size - 1;
    
    for (int i = 0; i < size; i++) {
        sum += *p;  /* *(p + 0) */
        p--;        /* Candidate decrement instruction */
    }
    return sum;
}

int main() {
    /* Initialize test data */
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    float farr[N];
    struct Point points[N];
    char char_buf[N];
    int dst[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = 0;
        farr[i] = i * 1.5f;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
        char_buf[i] = (char)(i % 256);
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    /* Execute all patterns to ensure code is generated */
    int result = 0;
    
    result += pattern1_simple_pointer_postinc(arr, N);
    pattern2_indexed_access_postinc(buffer, N);
    result += pattern3_explicit_increment(arr, N, 1);
    pattern4_nested_invariant_base(matrix);
    result += (int)pattern5_float_traversal(farr, N);
    result += pattern6_struct_traversal(points, N);
    result += pattern7_char_traversal(char_buf, N);
    pattern8_multiple_refs(arr, dst, N);
    result += pattern9_while_loop(arr, N);
    result += pattern10_reverse_traversal(arr, N);
    
    printf("Result: %d (just to prevent dead code elimination)\n", result);
    
    return 0;
}
