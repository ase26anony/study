/* test_auto_inc_dec.c
 * Compile with: gcc -O1 -c test_auto_inc_dec.c -o test.o
 * Or: gcc -O2 -fno-unroll-loops -c test_auto_inc_dec.c -o test.o
 */

#define SIZE 100

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
void clear_buffer(int* buffer) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < SIZE; i++) {
        buffer[i] = 0;  /* After optimization: *(base + 0) where base = buffer + i*4 */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void process_matrix(int matrix[SIZE][SIZE]) {
    for (int row = 0; row < SIZE; row++) {
        int* base = &matrix[row][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 offset */
        for (int col = 0; col < SIZE; col++) {
            base[col] = row * SIZE + col;  /* base[col] = *(base + 0) during RTL expansion */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_every_other(const int* arr, int n) {
    int sum = 0;
    const int* ptr = arr;
    const int stride = 2;
    
    for (int i = 0; i < n; i += stride) {
        sum += *ptr;        /* *(ptr + 0) */
        ptr += stride;      /* Explicit increment by constant */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int a;
    int b;
    float c;
    char d[4];
};

void init_struct_array(struct Data* array, int n) {
    struct Data* ptr = array;
    
    for (int i = 0; i < n; i++) {
        ptr->a = i;        /* Access through pointer with offset 0 */
        ptr->b = i * 2;
        ptr->c = i * 1.5f;
        ptr++;             /* Increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_transform(int* dest, const int* src, int n) {
    const int* s = src;
    int* d = dest;
    
    for (int i = 0; i < n; i++) {
        *d = *s + 1;  /* Two memory accesses: *(d + 0) and *(s + 0) */
        s++;
        d++;
    }
}

/* Main function to ensure all patterns are used */
int main() {
    int arr[SIZE];
    int buffer[SIZE];
    int matrix[SIZE][SIZE];
    struct Data struct_arr[SIZE];
    int dest[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = 0;
    }
    
    /* Call all functions to ensure they're compiled */
    int result1 = sum_array(arr, SIZE);
    clear_buffer(buffer);
    process_matrix(matrix);
    int result2 = sum_every_other(arr, SIZE);
    init_struct_array(struct_arr, SIZE);
    copy_and_transform(dest, arr, SIZE);
    
    /* Use results to prevent dead code elimination */
    return result1 + result2 + dest[0] + struct_arr[0].a;
}
