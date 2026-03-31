/* test_auto_inc_dec.c
 * Compile with: gcc -O1 -c test_auto_inc_dec.c -o test.o
 * Or: gcc -O2 -fno-unroll-loops -c test_auto_inc_dec.c -o test.o
 */

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    
    /* This should generate: mem = *(p + 0), then p = p + 4 */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Access with offset 0 */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < N; i++) {
        buffer[i] = 0;  /* Initially base + (i * 4), may become ptr + 0 */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 offset */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(int *start, int stride) {
    int total = 0;
    int *ptr = start;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < N; i++) {
        total += *ptr;    /* ptr + 0 */
        ptr += stride;    /* Candidate increment instruction */
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

void process_struct_array(struct Data *array) {
    struct Data *ptr = array;
    
    /* Struct pointer increment creates larger constant stride */
    for (int i = 0; i < N; i++) {
        ptr->a = ptr->b + i;  /* Access at ptr + 0 */
        ptr++;                /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Multiple memory references in same loop */
void copy_arrays(int *src, int *dst, int size) {
    int *s = src;
    int *d = dst;
    
    /* Two memory references, both with offset 0 */
    for (int i = 0; i < size; i++) {
        *d = *s;  /* Both *(s + 0) and *(d + 0) */
        s++;
        d++;
    }
}

/* Pattern 7: Loop with pointer increment in middle */
void modify_array(int *arr, int size) {
    int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        int val = *ptr;      /* Load with offset 0 */
        val = val * 2 + 1;
        *ptr = val;          /* Store with offset 0 */
        ptr++;               /* Increment after both accesses */
    }
}

/* Pattern 8: char pointer for single-byte stride */
int count_chars(const char *str, char target) {
    const char *p = str;
    int count = 0;
    
    while (*p != '\0') {
        if (*p == target) {  /* *(p + 0) */
            count++;
        }
        p++;  /* Increment by 1 */
    }
    return count;
}

/* Main function to ensure all patterns are used */
int main() {
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    struct Data struct_arr[N];
    char str[] = "test_string_for_coverage";
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = 0;
    }
    
    /* Exercise all patterns */
    int sum1 = sum_array(arr, N);
    clear_buffer(buffer);
    fill_matrix(matrix);
    int sum2 = sum_with_stride(arr, 2);
    process_struct_array(struct_arr);
    
    int src[N], dst[N];
    for (int i = 0; i < N; i++) src[i] = i;
    copy_arrays(src, dst, N);
    
    modify_array(arr, N);
    int char_count = count_chars(str, 't');
    
    /* Return something based on computations to avoid dead code elimination */
    return sum1 + sum2 + char_count + arr[0] + dst[0];
}
