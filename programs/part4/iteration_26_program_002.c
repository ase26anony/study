/* test_auto_inc_dec.c
 * Designed to trigger the uncovered lines in auto-inc-dec.cc:
 *   mem_insn.reg1_is_const = true;
 *   mem_insn.reg1_val = 0;
 *   if (find_inc (true)) return true;
 *
 * Compile with: gcc -O1 -c test_auto_inc_dec.c -o test.o
 * Or with: gcc -O2 -fno-unroll-loops -c test_auto_inc_dec.c -o test.o
 */

#define SIZE 100

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int* arr, int n) {
    int sum = 0;
    const int* p = arr;
    
    /* Classic *ptr++ pattern - access uses *(ptr + 0), then ptr is incremented */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* Should become base + 0 addressing */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int* buffer, int n) {
    /* This often gets transformed to pointer arithmetic with zero offset */
    for (int i = 0; i < n; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Explicit pointer arithmetic with stride */
int sum_with_stride(const int* arr, int n, int stride) {
    int sum = 0;
    const int* ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < n; i++) {
        sum += *ptr;    /* base + 0 */
        ptr += stride;  /* candidate increment instruction */
    }
    return sum;
}

/* Pattern 4: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[][SIZE], int rows, int cols) {
    for (int j = 0; j < rows; j++) {
        int* base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < cols; i++) {
            base[i] = i * j;  /* Should become *(base + 0) after optimization */
        }
    }
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
    
    /* Struct access often creates clean pointer increment patterns */
    for (int i = 0; i < n; i++) {
        ptr->a = i;     /* base + 0 access */
        ptr->b = i * 2;
        ptr++;          /* increment by sizeof(struct Data) */
    }
}

/* Pattern 6: Multiple memory references in same loop */
void copy_and_scale(int* dest, const int* src, int n, int factor) {
    const int* s = src;
    int* d = dest;
    
    /* Two memory references, both should be base + 0 */
    for (int i = 0; i < n; i++) {
        *d = *s * factor;  /* Two base + 0 accesses */
        s++;
        d++;
    }
}

/* Pattern 7: char pointer for single-byte increments */
int count_chars(const char* str, char target) {
    int count = 0;
    const char* p = str;
    
    while (*p != '\0') {
        if (*p == target) {  /* base + 0 access */
            count++;
        }
        p++;  /* increment by 1 */
    }
    return count;
}

/* Pattern 8: Loop with if condition that doesn't prevent optimization */
int sum_positive(const int* arr, int n) {
    int sum = 0;
    const int* p = arr;
    
    for (int i = 0; i < n; i++) {
        if (*p > 0) {  /* base + 0 access */
            sum += *p; /* another base + 0 access */
        }
        p++;
    }
    return sum;
}

/* Main function to ensure all patterns are used */
int main() {
    int arr[SIZE];
    int buffer[SIZE];
    int matrix[SIZE][SIZE];
    struct Data struct_arr[SIZE];
    int dest[SIZE];
    const char* test_str = "Hello, World!";
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = 0;
        struct_arr[i].a = i;
        struct_arr[i].b = i * 2;
    }
    
    /* Call all functions to ensure they're not optimized away */
    int result1 = sum_array(arr, SIZE);
    clear_buffer(buffer, SIZE);
    int result2 = sum_with_stride(arr, SIZE, 1);
    fill_matrix(matrix, SIZE/10, SIZE/10);
    process_struct_array(struct_arr, SIZE);
    copy_and_scale(dest, arr, SIZE, 2);
    int result3 = count_chars(test_str, 'l');
    int result4 = sum_positive(arr, SIZE);
    
    /* Return sum of all results to prevent dead code elimination */
    return result1 + result2 + result3 + result4;
}
