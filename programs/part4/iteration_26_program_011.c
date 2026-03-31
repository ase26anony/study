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
    const int *p = arr;
    
    /* This should generate: mem = *(p + 0), then p = p + 4 */
    for (int i = 0; i < size; i++) {
        sum += *p;      /* Access with base + 0 offset */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer */
void clear_buffer(int *buffer, int size) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Nested loops with invariant base */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base calculated in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_every_other(const int *arr, int size, int stride) {
    int sum = 0;
    const int *ptr = arr;
    const int *end = arr + size;
    
    /* Explicit increment separate from access */
    while (ptr < end) {
        sum += *ptr;        /* *(ptr + 0) */
        ptr += stride;      /* Candidate increment for find_inc */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int a;
    int b;
    float c;
    double d;
};

void process_structs(struct Data *data, int count) {
    struct Data *ptr = data;
    
    /* Large stride (24 bytes on typical 64-bit) */
    for (int i = 0; i < count; i++) {
        ptr->a = i;         /* Access at ptr + 0 */
        ptr->b = i * 2;
        ptr++;              /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Mixed access patterns in same loop */
void mixed_access(char *buf, int *ints, float *floats, int n) {
    char *cptr = buf;
    int *iptr = ints;
    float *fptr = floats;
    
    for (int i = 0; i < n; i++) {
        /* Multiple base+0 accesses with different types */
        *cptr = (char)i;    /* char* access */
        cptr++;             /* Increment by 1 */
        
        *iptr = i;          /* int* access */
        iptr++;             /* Increment by 4 */
        
        *fptr = (float)i;   /* float* access */
        fptr++;             /* Increment by 4 */
    }
}

/* Pattern 7: Loop with if condition that doesn't prevent optimization */
int conditional_sum(const int *arr, int size, int threshold) {
    int sum = 0;
    const int *p = arr;
    
    for (int i = 0; i < size; i++) {
        int val = *p;       /* Access with p + 0 */
        if (val > threshold) {
            sum += val;
        }
        p++;                /* Increment always happens */
    }
    return sum;
}

int main() {
    /* Initialize test data */
    int array[N];
    int buffer[N];
    int matrix[M][N];
    struct Data structs[N/4];
    char char_buf[N];
    int int_array[N];
    float float_array[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array[i] = i;
        buffer[i] = i * 2;
        int_array[i] = i * 3;
        float_array[i] = i * 0.5f;
        char_buf[i] = (char)i;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    for (int i = 0; i < N/4; i++) {
        structs[i].a = i;
        structs[i].b = i * 2;
        structs[i].c = i * 0.25f;
        structs[i].d = i * 0.125;
    }
    
    /* Execute all patterns */
    int sum1 = sum_array(array, N);
    clear_buffer(buffer, N);
    fill_matrix(matrix);
    int sum2 = sum_every_other(array, N, 2);
    process_structs(structs, N/4);
    mixed_access(char_buf, int_array, float_array, N/2);
    int sum3 = conditional_sum(array, N, 100);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d %d %d\n", sum1, sum2, sum3);
    printf("Buffer[10] = %d\n", buffer[10]);
    printf("Matrix[5][10] = %d\n", matrix[5][10]);
    printf("Structs[3].a = %d\n", structs[3].a);
    
    return 0;
}
