/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto_inc_dec optimization pass
 * Specifically targets the find_inc(true) path with reg1_val = 0
 */

#define SIZE 1024
#define ITER 1000000

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* This should generate: load from (p + 0), then p += 4 */
    while (p < end) {
        sum += *p;      /* Access at offset 0 */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access that gets optimized to pointer */
void clear_buffer(int *buffer, int n) {
    int i;
    
    /* Compiler's ivopts should convert this to pointer form */
    for (i = 0; i < n; i++) {
        buffer[i] = 0;  /* Initially base + (i*4), optimized to base + 0 */
    }
}

/* Pattern 3: Nested loops with invariant base */
#define ROWS 128
#define COLS 128
void process_matrix(int matrix[ROWS][COLS]) {
    int i, j;
    
    for (j = 0; j < ROWS; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (i = 0; i < COLS; i++) {
            base[i] = i * j;  /* Should become *(base + 0) then base += 4 */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(int *arr, int n, int stride) {
    int sum = 0;
    int *ptr = arr;
    int count = n;
    
    /* Explicit increment separate from access */
    while (count-- > 0) {
        sum += *ptr;    /* Access at offset 0 */
        ptr += stride;  /* Separate increment by constant */
    }
    return sum;
}

/* Pattern 5: Struct access to ensure proper alignment */
struct element {
    int value;
    int tag;
};

int sum_struct_array(struct element *arr, int n) {
    int sum = 0;
    struct element *p = arr;
    struct element *end = arr + n;
    
    while (p < end) {
        sum += p->value;  /* Access at offset 0 within struct */
        p++;              /* Increment by sizeof(struct element) */
    }
    return sum;
}

/* Pattern 6: Char pointer for byte access */
void memset_pattern(char *buf, char pattern, int n) {
    char *p = buf;
    char *end = buf + n;
    
    while (p < end) {
        *p = pattern;  /* Store at offset 0 */
        p++;           /* Increment by 1 */
    }
}

/* Main function to exercise all patterns */
int main() {
    int array[SIZE];
    int buffer[SIZE];
    int matrix[ROWS][COLS];
    struct element struct_arr[SIZE];
    char char_buf[SIZE];
    int i;
    
    /* Initialize data */
    for (i = 0; i < SIZE; i++) {
        array[i] = i;
        buffer[i] = i * 2;
        struct_arr[i].value = i;
        struct_arr[i].tag = i % 10;
        char_buf[i] = 'A' + (i % 26);
    }
    
    for (i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }
    
    /* Exercise different patterns */
    int result1 = sum_array(array, SIZE);
    clear_buffer(buffer, SIZE);
    process_matrix(matrix);
    int result2 = sum_with_stride(array, SIZE, 1);
    int result3 = sum_struct_array(struct_arr, SIZE);
    memset_pattern(char_buf, 'X', SIZE);
    
    /* Use results to prevent dead code elimination */
    return result1 + result2 + result3 + buffer[0] + matrix[0][0] + char_buf[0];
}
