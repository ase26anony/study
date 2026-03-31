/* test_auto_inc_dec.c
 * Compile with: gcc -O1 -c test_auto_inc_dec.c -o test.o
 * Or: gcc -O2 -fno-unroll-loops -c test_auto_inc_dec.c -o test.o
 */

#define SIZE 100
#define INNER_SIZE 50
#define STRIDE 4

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int* arr, int n) {
    int sum = 0;
    const int* p = arr;
    for (int i = 0; i < n; i++) {
        sum += *p;  /* Access with base + 0 */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int* buffer, int n) {
    for (int i = 0; i < n; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[][INNER_SIZE], int rows) {
    for (int j = 0; j < rows; j++) {
        int* base = &matrix[j][0];  /* Base computed in outer loop */
        for (int i = 0; i < INNER_SIZE; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit pointer arithmetic with stride */
int sum_with_stride(const int* start, int n, int stride) {
    int total = 0;
    const int* ptr = start;
    for (int i = 0; i < n; i++) {
        total += *ptr;      /* base + 0 access */
        ptr += stride;      /* explicit increment by constant */
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
    for (int i = 0; i < n; i++) {
        array[i].a = i;     /* May create pointer with offset 0 */
        array[i].b = i * 2;
    }
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_increment(int* dest, const int* src, int n) {
    const int* s = src;
    int* d = dest;
    for (int i = 0; i < n; i++) {
        *d = *s;    /* Two base+0 accesses */
        s++;        /* Two separate increments */
        d++;
    }
}

/* Pattern 7: char pointer for single-byte stride */
int count_chars(const char* str, char target) {
    int count = 0;
    const char* p = str;
    while (*p != '\0') {
        if (*p == target) count++;  /* base + 0 access */
        p++;                        /* increment by 1 */
    }
    return count;
}

/* Pattern 8: Loop with if condition that doesn't prevent optimization */
void conditional_store(int* data, int n, int threshold) {
    int* ptr = data;
    for (int i = 0; i < n; i++) {
        if (i > threshold) {
            *ptr = i;   /* base + 0 in conditional path */
        }
        ptr++;          /* increment always executed */
    }
}

/* Main function to ensure all patterns are used */
int main() {
    int arr[SIZE];
    int buffer[SIZE];
    int matrix[SIZE][INNER_SIZE];
    struct Data struct_arr[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = 0;
        struct_arr[i].a = 0;
        struct_arr[i].b = 0;
    }
    
    /* Execute all patterns to ensure code generation */
    int result1 = sum_array(arr, SIZE);
    clear_buffer(buffer, SIZE);
    fill_matrix(matrix, SIZE / 2);
    int result2 = sum_with_stride(arr, SIZE, STRIDE);
    process_struct_array(struct_arr, SIZE);
    
    int dest[SIZE];
    copy_and_increment(dest, arr, SIZE);
    
    char str[] = "test string for auto increment optimization";
    int result3 = count_chars(str, 't');
    
    conditional_store(arr, SIZE, SIZE / 2);
    
    /* Return sum of results to prevent dead code elimination */
    return result1 + result2 + result3 + dest[0] + arr[0];
}
