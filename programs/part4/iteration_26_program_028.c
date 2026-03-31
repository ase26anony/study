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
        sum += *p;  /* Access with offset 0 */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer, int n) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < n; i++) {
        buffer[i] = 0;  /* After optimization: *(base + i*4) -> *(ptr + 0) */
    }
}

/* Pattern 3: Pointer arithmetic with explicit stride */
void stride_access(int *data, int n, int stride) {
    int *ptr = data;
    /* Force separate increment instruction */
    for (int i = 0; i < n; i++) {
        *ptr = i;           /* *(ptr + 0) */
        ptr += stride;      /* Candidate increment for find_inc */
    }
}

/* Pattern 4: Nested loops with invariant base in inner loop */
void matrix_init(int matrix[][SIZE], int rows, int cols) {
    for (int j = 0; j < rows; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 offset */
        for (int i = 0; i < cols; i++) {
            base[i] = i * j;  /* After ivopts: *(ptr + 0) where ptr increments */
        }
    }
}

/* Pattern 5: Struct access to ensure proper alignment */
struct Point {
    int x;
    int y;
    int z;
};

int sum_points(struct Point *points, int n) {
    int total = 0;
    struct Point *p = points;
    
    /* Struct access often prevents other optimizations that might
       interfere with auto_inc_dec pattern */
    for (int i = 0; i < n; i++) {
        total += p->x + p->y;  /* Access with offset 0 */
        p++;                   /* Increment by sizeof(struct Point) */
    }
    return total;
}

/* Pattern 6: Character array to test byte increments */
void copy_string(char *dest, const char *src, int len) {
    char *d = dest;
    const char *s = src;
    
    /* Byte accesses with increment of 1 */
    for (int i = 0; i < len; i++) {
        *d = *s;  /* Both accesses with offset 0 */
        d++;
        s++;
    }
}

/* Pattern 7: Mixed patterns to increase coverage */
void mixed_patterns(int *a, int *b, int n) {
    int *pa = a;
    int *pb = b;
    
    /* Two memory accesses with zero offset, each with their own increment */
    for (int i = 0; i < n; i++) {
        *pa = *pb;  /* Both *(ptr + 0) */
        pa += 2;    /* Increment by 8 bytes */
        pb++;       /* Increment by 4 bytes */
    }
}

/* Pattern 8: Loop with if condition that doesn't prevent auto-inc-dec */
void conditional_store(int *data, int *mask, int n) {
    int *p = data;
    int *m = mask;
    
    for (int i = 0; i < n; i++) {
        if (*m > 0) {  /* Access with offset 0 */
            *p = 1;    /* Access with offset 0 */
        }
        p++;
        m++;
    }
}

/* Main function to ensure all patterns are used */
int main() {
    int arr[SIZE];
    int buffer[SIZE];
    int matrix[SIZE][SIZE];
    struct Point points[SIZE];
    char str1[SIZE], str2[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = 0;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
        str1[i] = 'A' + (i % 26);
    }
    
    /* Call all patterns to ensure they're compiled */
    int result = 0;
    result += sum_array(arr, SIZE);
    clear_buffer(buffer, SIZE);
    stride_access(arr, SIZE, 1);
    matrix_init(matrix, SIZE/10, SIZE/10);
    result += sum_points(points, SIZE);
    copy_string(str2, str1, SIZE);
    mixed_patterns(arr, buffer, SIZE/2);
    conditional_store(arr, buffer, SIZE);
    
    return result % 256;  /* Prevent dead code elimination */
}
