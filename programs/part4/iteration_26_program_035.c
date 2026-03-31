/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto_inc_dec optimization pass
 * Specifically targets the find_inc(true) path with reg1_val = 0
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *ptr = arr;
    
    /* This should generate: mem = *(ptr + 0), then ptr = ptr + 4 */
    for (int i = 0; i < size; i++) {
        sum += *ptr;    /* Access with base + 0 offset */
        ptr++;          /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access that gets optimized to pointer */
void clear_buffer(int *buffer, int size) {
    /* Compiler's ivopts should convert this to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* Should become *(base + 0) after optimization */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Should become *(base + 0) initially */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(const int *arr, int size, int stride) {
    int sum = 0;
    const int *ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < size; i++) {
        sum += *ptr;        /* *(ptr + 0) */
        ptr += stride;      /* Candidate increment for find_inc */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure proper alignment */
struct Data {
    int a;
    int b;
    float c;
};

void process_structs(struct Data *data, int count) {
    struct Data *ptr = data;
    
    /* Access struct members - compiler may use base + 0 for ptr->a */
    for (int i = 0; i < count; i++) {
        ptr->a = i;         /* Should be *(ptr + 0) for first member */
        ptr->b = i * 2;
        ptr->c = i * 3.0f;
        ptr++;              /* Separate increment */
    }
}

/* Pattern 6: Character buffer processing */
void toupper_buffer(char *str) {
    char *p = str;
    
    while (*p) {
        /* Access with *(p + 0), then increment */
        if (*p >= 'a' && *p <= 'z') {
            *p = *p - ('a' - 'A');
        }
        p++;  /* Separate increment instruction */
    }
}

/* Pattern 7: Multiple arrays with same index */
void copy_arrays(const int *src, int *dst, int size) {
    const int *s = src;
    int *d = dst;
    
    for (int i = 0; i < size; i++) {
        *d = *s;    /* Two memory accesses, both with base + 0 */
        s++;
        d++;        /* Two separate increments */
    }
}

/* Pattern 8: Loop with if condition that doesn't prevent optimization */
int conditional_sum(const int *arr, int size, int threshold) {
    int sum = 0;
    const int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        int val = *ptr;  /* *(ptr + 0) */
        if (val > threshold) {
            sum += val;
        }
        ptr++;  /* Increment after the access */
    }
    return sum;
}

int main() {
    /* Initialize test data */
    int array[N];
    int buffer[N];
    int matrix[M][N];
    struct Data structs[N];
    char str[] = "test_string_for_auto_inc_dec";
    
    /* Fill with some values */
    for (int i = 0; i < N; i++) {
        array[i] = i;
        buffer[i] = i * 2;
        structs[i].a = i;
        structs[i].b = i * 2;
        structs[i].c = i * 3.0f;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    /* Execute all patterns to ensure code is generated */
    int sum1 = sum_array(array, N);
    clear_buffer(buffer, N);
    fill_matrix(matrix);
    int sum2 = sum_with_stride(array, N, 1);
    process_structs(structs, N);
    toupper_buffer(str);
    
    int dst[N];
    copy_arrays(array, dst, N);
    int sum3 = conditional_sum(array, N, N/2);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d %d %d\n", sum1, sum2, sum3);
    printf("String: %s\n", str);
    printf("Buffer[0] = %d, dst[0] = %d\n", buffer[0], dst[0]);
    
    return 0;
}
