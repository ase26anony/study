/* test_auto_inc_dec.c
 * Designed to trigger the uncovered lines in GCC's auto-inc-dec pass
 * where mem_insn.reg1_is_const = true and mem_insn.reg1_val = 0
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *ptr = arr;
    
    /* This should generate: load from (ptr + 0), then ptr += 4 */
    for (int i = 0; i < size; i++) {
        sum += *ptr;    /* Access at offset 0 */
        ptr++;          /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer, int size) {
    /* Classic pattern that ivopts transforms to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* After ivopts: *(base + 0) where base = buffer + i*4 */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(const int *arr, int size, int stride) {
    int sum = 0;
    const int *ptr = arr;
    
    /* Force separate increment instruction */
    for (int i = 0; i < size; i++) {
        sum += *ptr;        /* *(ptr + 0) */
        ptr += stride;      /* Candidate increment for find_inc */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure proper scaling */
struct Data {
    int a;
    int b;
    float c;
};

void process_structs(struct Data *data, int count) {
    struct Data *ptr = data;
    
    /* Struct access often generates base + offset addressing */
    for (int i = 0; i < count; i++) {
        ptr->a = i;         /* Should be *(ptr + 0) for first member */
        ptr->b = i * 2;
        ptr++;              /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Char pointer for byte-granular access */
int count_chars(const char *str) {
    int count = 0;
    const char *p = str;
    
    while (*p != '\0') {
        if (*p == 'a') {    /* *(p + 0) */
            count++;
        }
        p++;                /* Increment by 1 */
    }
    return count;
}

/* Pattern 7: Mixed access patterns in same loop */
void mixed_access(int *dest, const int *src1, const int *src2, int size) {
    int *d = dest;
    const int *s1 = src1;
    const int *s2 = src2;
    
    for (int i = 0; i < size; i++) {
        *d = *s1 + *s2;    /* Three separate *(ptr + 0) accesses */
        d++;
        s1++;
        s2++;               /* Three candidate increments */
    }
}

/* Pattern 8: Loop with if condition that doesn't prevent optimization */
void conditional_store(int *arr, int size, int threshold) {
    int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        if (i > threshold) {
            *ptr = i;       /* *(ptr + 0) in conditional path */
        }
        ptr++;              /* Increment always executed */
    }
}

/* Main function to exercise all patterns */
int main() {
    int arr[N];
    int matrix[M][N];
    struct Data structs[100];
    char str[] = "test string with some a characters";
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    for (int i = 0; i < 100; i++) {
        structs[i].a = 0;
        structs[i].b = 0;
        structs[i].c = 0.0f;
    }
    
    /* Exercise each pattern */
    int sum1 = sum_array(arr, N);
    printf("Sum1: %d\n", sum1);
    
    clear_buffer(arr, N);
    
    fill_matrix(matrix);
    printf("Matrix[0][0]: %d\n", matrix[0][0]);
    
    int sum2 = sum_with_stride(arr, N/2, 2);
    printf("Sum2: %d\n", sum2);
    
    process_structs(structs, 100);
    printf("Struct[0].a: %d\n", structs[0].a);
    
    int count = count_chars(str);
    printf("Char count: %d\n", count);
    
    int src1[N], src2[N], dest[N];
    for (int i = 0; i < N; i++) {
        src1[i] = i;
        src2[i] = N - i;
    }
    mixed_access(dest, src1, src2, N);
    printf("Mixed[0]: %d\n", dest[0]);
    
    conditional_store(arr, N, N/2);
    printf("Conditional[%d]: %d\n", N/2 + 1, arr[N/2 + 1]);
    
    return 0;
}
