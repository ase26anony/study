/* test_auto_inc_dec.c
 * Designed to trigger the uncovered lines in auto-inc-dec.cc:
 *   mem_insn.reg1_is_const = true;
 *   mem_insn.reg1_val = 0;
 *   if (find_inc (true)) return true;
 */

#define SIZE 100
#define INNER_SIZE 50

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int* arr, int n) {
    int sum = 0;
    const int* p = arr;
    
    /* This should generate: mem = *(p + 0), then p = p + 4 */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* Access with base + 0 offset */
        p++;            /* Separate increment instruction */
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
void fill_matrix(int matrix[][INNER_SIZE], int rows) {
    for (int j = 0; j < rows; j++) {
        int* base = &matrix[j][0];  /* Base calculated in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < INNER_SIZE; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
int sum_with_stride(const int* start, int stride, int n) {
    int total = 0;
    const int* ptr = start;
    
    for (int i = 0; i < n; i++) {
        total += *ptr;      /* *(ptr + 0) */
        ptr += stride;      /* Explicit increment by constant */
    }
    return total;
}

/* Pattern 5: Struct access to ensure proper scaling */
struct Data {
    int values[4];
    char tag;
};

int sum_struct_array(const struct Data* array, int n) {
    int sum = 0;
    const struct Data* p = array;
    
    for (int i = 0; i < n; i++) {
        /* Access each field - creates multiple memory references */
        sum += p->values[0];
        sum += p->values[2];
        p++;  /* Increment by sizeof(struct Data) */
    }
    return sum;
}

/* Pattern 6: Mixed access patterns to stress the optimization */
void mixed_access(int* dest, const int* src1, const int* src2, int n) {
    int* d = dest;
    const int* s1 = src1;
    const int* s2 = src2;
    
    for (int i = 0; i < n; i++) {
        *d = *s1 + *s2;  /* Three memory accesses, all base + 0 */
        d++;
        s1++;
        s2++;
    }
}

/* Main function to ensure all patterns are used */
int main() {
    int arr[SIZE];
    int buffer[SIZE];
    int matrix[10][INNER_SIZE];
    struct Data struct_arr[20];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = i * 2;
    }
    
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 4; j++) {
            struct_arr[i].values[j] = i + j;
        }
    }
    
    /* Execute all patterns */
    int sum1 = sum_array(arr, SIZE);
    clear_buffer(buffer, SIZE);
    fill_matrix(matrix, 10);
    int sum2 = sum_with_stride(arr, 2, SIZE/2);
    int sum3 = sum_struct_array(struct_arr, 20);
    
    int dest[SIZE];
    mixed_access(dest, arr, buffer, SIZE);
    
    /* Use results to prevent dead code elimination */
    return sum1 + sum2 + sum3 + dest[0];
}
