/* test_auto_inc_dec.c
 * Designed to trigger the uncovered lines in GCC's auto_inc_dec pass
 * where mem_insn.reg1_val = 0 and find_inc(true) returns true.
 */

#define SIZE 100
#define INNER_SIZE 50

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int* arr, int n) {
    int sum = 0;
    const int* p = arr;
    
    /* This should generate: mem = *(p + 0), then p = p + 4 */
    for (int i = 0; i < n; i++) {
        sum += *p;  /* Access with offset 0 */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int* buffer, int n) {
    /* Classic pattern that often gets transformed to pointer arithmetic */
    for (int i = 0; i < n; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[][INNER_SIZE], int rows) {
    for (int j = 0; j < rows; j++) {
        int* base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 offset */
        for (int i = 0; i < INNER_SIZE; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(const int* start, int stride, int n) {
    int total = 0;
    const int* ptr = start;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < n; i++) {
        total += *ptr;    /* *(ptr + 0) */
        ptr += stride;    /* Candidate increment for find_inc */
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

void process_struct_array(struct Data* array, int n) {
    struct Data* ptr = array;
    
    /* Access struct with pointer, increment by sizeof(struct Data) */
    for (int i = 0; i < n; i++) {
        ptr->a = i;      /* Access through pointer with offset 0 */
        ptr->b = i * 2;
        ptr++;           /* Increment by struct size */
    }
}

/* Pattern 6: Mixed access patterns to test different optimization paths */
void mixed_access(int* dest, const int* src1, const int* src2, int n) {
    int* d = dest;
    const int* s1 = src1;
    const int* s2 = src2;
    
    for (int i = 0; i < n; i++) {
        /* Multiple memory accesses with potential auto-inc opportunities */
        *d = *s1 + *s2;  /* Three *(ptr + 0) accesses */
        d++;
        s1++;
        s2++;
    }
}

/* Pattern 7: Loop with pointer increment in separate statement */
void copy_reverse(const int* src, int* dst, int n) {
    const int* s = src;
    int* d = &dst[n - 1];
    
    for (int i = 0; i < n; i++) {
        *d = *s;  /* Access with offset 0 */
        s = s + 1;  /* Increment in separate statement */
        d = d - 1;  /* Decrement candidate for find_inc */
    }
}

/* Main function to ensure all patterns are used */
int main() {
    int arr[SIZE];
    int buffer[SIZE];
    int matrix[SIZE][INNER_SIZE];
    int dest[SIZE];
    struct Data struct_arr[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = 0;
    }
    
    /* Test all patterns */
    int sum1 = sum_array(arr, SIZE);
    clear_buffer(buffer, SIZE);
    fill_matrix(matrix, SIZE / 2);
    int sum2 = sum_with_stride(arr, 2, SIZE / 2);
    process_struct_array(struct_arr, SIZE);
    mixed_access(dest, arr, buffer, SIZE);
    copy_reverse(arr, dest, SIZE);
    
    /* Use results to prevent dead code elimination */
    return sum1 + sum2 + dest[0] + struct_arr[0].a;
}
