/* test_auto_inc_dec.c
 * Compile with: gcc -O1 -c test_auto_inc_dec.c -o test.o
 * Or: gcc -O2 -fno-unroll-loops -c test_auto_inc_dec.c -o test.o
 */

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int* arr, int size) {
    int sum = 0;
    const int* p = arr;
    for (int i = 0; i < size; i++) {
        sum += *p;      /* Should become mem_insn with base + 0 */
        p++;            /* Candidate increment for find_inc() */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int* buffer) {
    for (int i = 0; i < N; i++) {
        buffer[i] = 0;  /* GIMPLE: *(buffer + (i * 4)) */
                        /* After ivopts: *(ptr + 0) with ptr increment elsewhere */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int* base = &matrix[j][0];  /* Base computed in outer loop */
        for (int i = 0; i < N; i++) {
            base[i] = i;            /* Inner loop: *(base + 0) initially */
                                    /* Separate increment of index or pointer */
        }
    }
}

/* Pattern 4: Explicit pointer arithmetic with stride */
void process_with_stride(char* data, int stride) {
    char* ptr = data;
    for (int i = 0; i < N; i++) {
        *ptr = 'A';     /* *(ptr + 0) */
        ptr += stride;  /* Explicit increment instruction */
    }
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Point {
    int x;
    int y;
    int z;
};

int sum_points(struct Point* points, int count) {
    int total = 0;
    struct Point* p = points;
    for (int i = 0; i < count; i++) {
        total += p->x;  /* Access at offset 0 within struct */
        p++;            /* Increment by sizeof(struct Point) */
    }
    return total;
}

/* Pattern 6: Multiple memory references with same base */
void copy_swap(int* dest, const int* src, int n) {
    const int* s = src;
    int* d = dest;
    for (int i = 0; i < n; i++) {
        *d = *s;        /* Two memory refs, both *(ptr + 0) */
        d++;
        s++;
    }
}

/* Pattern 7: Loop with pointer increment in separate statement */
void accumulate(float* values, float* result, int n) {
    float* v = values;
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += *v;      /* Memory access with zero offset */
        v = v + 1;      /* Increment in separate statement */
    }
    *result = sum;
}

/* Pattern 8: char pointer for single-byte stride */
int count_chars(const char* str, char target) {
    int count = 0;
    const char* p = str;
    while (*p != '\0') {
        if (*p == target) count++;  /* *(p + 0) */
        p++;                        /* Increment by 1 */
    }
    return count;
}

/* Main function to ensure all patterns are used */
int main() {
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    char data[N];
    struct Point points[N];
    int dest[N], src[N];
    float values[N], result;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        src[i] = i * 2;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = 0;
        }
    }
    
    /* Call all functions to ensure they're compiled */
    int sum1 = sum_array(arr, N);
    clear_buffer(buffer);
    fill_matrix(matrix);
    process_with_stride(data, 1);
    int sum2 = sum_points(points, N);
    copy_swap(dest, src, N);
    accumulate(values, &result, N);
    int count = count_chars("test string", 't');
    
    /* Prevent dead code elimination */
    return sum1 + sum2 + count + (int)result;
}
