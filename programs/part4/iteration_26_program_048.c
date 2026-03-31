/* test_auto_inc_dec.c
 * Designed to trigger the uncovered lines in auto-inc-dec.cc:
 *   mem_insn.reg1_is_const = true;
 *   mem_insn.reg1_val = 0;
 *   if (find_inc (true)) return true;
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate: mem = *(p + 0), then p = p + 4 */
    for (int i = 0; i < n; i++) {
        sum += *p;      /* Access with base + 0 offset */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer, int n) {
    /* Compiler's ivopts may transform this to pointer arithmetic */
    for (int i = 0; i < n; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Pointer with explicit stride */
void process_with_stride(int *data, int n, int stride) {
    int *ptr = data;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < n; i++) {
        *ptr = i * 2;   /* *(ptr + 0) */
        ptr += stride;  /* Candidate increment instruction */
    }
}

/* Pattern 4: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[][SIZE], int rows, int cols) {
    for (int r = 0; r < rows; r++) {
        int *base = &matrix[r][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int c = 0; c < cols; c++) {
            base[c] = r * cols + c;  /* May become *(base + 0 + c*4) */
        }
    }
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Point {
    int x;
    int y;
    int z;
};

int sum_points(struct Point *points, int n) {
    struct Point *p = points;
    int total = 0;
    
    /* Larger stride (12 bytes for 3 ints) */
    for (int i = 0; i < n; i++) {
        total += p->x + p->y + p->z;  /* Multiple *(ptr + 0) accesses */
        p++;                          /* Increment by 12 bytes */
    }
    return total;
}

/* Pattern 6: Character buffer processing */
void toupper_buffer(char *str, int len) {
    char *p = str;
    
    /* Small stride (1 byte) */
    for (int i = 0; i < len; i++) {
        if (*p >= 'a' && *p <= 'z')  /* *(p + 0) */
            *p = *p - ('a' - 'A');   /* Another *(p + 0) */
        p++;                         /* Increment by 1 */
    }
}

/* Pattern 7: Loop with multiple memory references */
void copy_and_transform(int *src, int *dst, int n) {
    int *s = src;
    int *d = dst;
    
    for (int i = 0; i < n; i++) {
        int val = *s;          /* *(s + 0) */
        *d = val * 2 + 1;      /* *(d + 0) */
        s++;                   /* Increment for src */
        d++;                   /* Increment for dst */
    }
}

/* Pattern 8: Do-while loop variant */
int find_value(const int *arr, int n, int target) {
    const int *p = arr;
    int count = 0;
    
    if (n <= 0) return -1;
    
    do {
        if (*p == target)      /* *(p + 0) */
            return count;
        p++;                   /* Increment */
        count++;
    } while (count < n);
    
    return -1;
}

/* Main function to exercise all patterns */
int main() {
    int arr[SIZE];
    int buffer[SIZE];
    int matrix[10][SIZE];
    struct Point points[SIZE];
    char str[] = "test_string_for_auto_inc_dec";
    
    /* Initialize test data */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
        buffer[i] = 0;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
    }
    
    /* Exercise each pattern */
    int sum = sum_array(arr, SIZE);
    printf("Sum: %d\n", sum);
    
    clear_buffer(buffer, SIZE);
    printf("Buffer[0]: %d\n", buffer[0]);
    
    process_with_stride(arr, SIZE, 1);
    printf("Processed arr[0]: %d\n", arr[0]);
    
    fill_matrix(matrix, 10, SIZE);
    printf("Matrix[0][0]: %d\n", matrix[0][0]);
    
    int point_sum = sum_points(points, SIZE);
    printf("Point sum: %d\n", point_sum);
    
    toupper_buffer(str, sizeof(str) - 1);
    printf("Uppercase: %s\n", str);
    
    int dst[SIZE];
    copy_and_transform(arr, dst, SIZE);
    printf("Copy result: %d\n", dst[0]);
    
    int found = find_value(arr, SIZE, 42);
    printf("Found 42 at: %d\n", found);
    
    return 0;
}
