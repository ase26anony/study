/* test_auto_inc_dec.c
 * Compile with: gcc -O1 -c test_auto_inc_dec.c -o test.o
 * Or: gcc -O2 -fno-unroll-loops -c test_auto_inc_dec.c -o test.o
 */

#define SIZE 100

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* This should generate: mem = *(p + 0), then p = p + 4 */
    for (int i = 0; i < n; i++) {
        sum += *p;  /* Access with base + 0 offset */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer induction */
void clear_buffer(char *buffer, int n) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < n; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void process_matrix(int matrix[][SIZE], int rows, int cols) {
    for (int j = 0; j < rows; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < cols; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(int *arr, int n, int stride) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n * stride;
    
    /* Explicit increment separate from access */
    while (ptr < end) {
        sum += *ptr;      /* *(ptr + 0) */
        ptr += stride;    /* Candidate increment for find_inc */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int a;
    int b;
    float c;
};

void init_struct_array(struct Data *arr, int n) {
    struct Data *p = arr;
    
    /* Struct pointer increment */
    for (int i = 0; i < n; i++) {
        p->a = i;        /* Access at p + 0 */
        p->b = i * 2;
        p->c = i * 3.0f;
        p++;             /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Mixed access patterns to avoid over-optimization */
void mixed_access(int *a, int *b, int n) {
    int *pa = a;
    int *pb = b;
    
    for (int i = 0; i < n; i++) {
        /* Two memory accesses with same base+0 pattern */
        *pa = *pb;      /* Both should be *(ptr + 0) */
        pa++;
        pb++;
    }
}

/* Main function to ensure code is not dead */
int main() {
    int arr[SIZE];
    char buffer[SIZE];
    int matrix[SIZE][SIZE];
    struct Data struct_arr[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = 'a' + (i % 26);
    }
    
    /* Call all functions to ensure they're compiled */
    int result = 0;
    result += sum_array(arr, SIZE);
    clear_buffer(buffer, SIZE);
    process_matrix(matrix, SIZE/2, SIZE/2);
    result += sum_with_stride(arr, SIZE, 2);
    init_struct_array(struct_arr, SIZE);
    mixed_access(arr, arr + SIZE/2, SIZE/2);
    
    return result % 256;  /* Prevent elimination of computation */
}
