/* test_auto_inc_dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec pass.
 * Compile with: gcc -O1 -c test_auto_inc_dec.c -o test.o
 * Or with: gcc -O2 -fno-unroll-loops -c test_auto_inc_dec.c -o test.o
 */

#define SIZE 100

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int* arr, int n) {
    int sum = 0;
    const int* p = arr;
    
    /* This should generate: mem = *(p + 0), then p = p + 4 */
    for (int i = 0; i < n; i++) {
        sum += *p;  /* Base + 0 access */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int* buffer, int n) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < n; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[][SIZE], int rows, int cols) {
    for (int j = 0; j < rows; j++) {
        int* base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < cols; i++) {
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
        ptr += stride;    /* Candidate increment instruction */
    }
    return total;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int values[4];
    char tag;
};

void process_struct_array(struct Data* array, int n) {
    struct Data* ptr = array;
    
    /* Larger stride may still trigger the pattern */
    for (int i = 0; i < n; i++) {
        ptr->tag = 'A';      /* Access at offset 0 within struct */
        ptr->values[0] = i;  /* Another base + 0 access */
        ptr++;               /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_transform(int* dest, const int* src, int n) {
    const int* s = src;
    int* d = dest;
    
    for (int i = 0; i < n; i++) {
        *d = *s + 1;  /* Two memory accesses: *(d + 0) and *(s + 0) */
        s++;          /* Increment source pointer */
        d++;          /* Increment destination pointer */
    }
}

/* Pattern 7: Loop with pointer arithmetic in condition */
int find_value(const int* arr, int n, int target) {
    const int* p = arr;
    const int* end = arr + n;
    
    /* While loop with pointer comparison */
    while (p < end) {
        if (*p == target)  /* *(p + 0) access */
            return 1;
        p++;  /* Separate increment */
    }
    return 0;
}

/* Pattern 8: Access with compile-time constant offset 0 */
void init_simple(int* data, int n) {
    int* ptr = data;
    
    /* Very simple pattern that should optimize well */
    for (int i = 0; i < n; i++) {
        ptr[0] = i;  /* Explicit [0] offset */
        ptr += 1;    /* Increment */
    }
}

/* Main function to ensure all patterns are used */
int main() {
    int arr[SIZE];
    int buffer[SIZE];
    int matrix[SIZE][SIZE];
    struct Data struct_arr[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = 0;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = 0;
        }
    }
    
    /* Exercise all patterns */
    int sum1 = sum_array(arr, SIZE);
    clear_buffer(buffer, SIZE);
    fill_matrix(matrix, SIZE, SIZE);
    int sum2 = sum_with_stride(arr, 2, SIZE/2);
    process_struct_array(struct_arr, SIZE);
    
    int dest[SIZE];
    copy_and_transform(dest, arr, SIZE);
    
    int found = find_value(arr, SIZE, 42);
    init_simple(buffer, SIZE);
    
    /* Return something based on computations to avoid dead code elimination */
    return sum1 + sum2 + found + dest[0] + buffer[0];
}
