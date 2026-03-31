/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns create memory accesses with base+0 addressing where
 * find_inc(true) can find a preceding increment instruction.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int *arr, int size) {
    int sum = 0;
    const int *p = arr;
    
    /* Classic *ptr++ pattern - access uses p+0, then p is incremented */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* Should become *(p + 0) after optimization */
        p++;        /* Separate increment instruction for find_inc to find */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int *buffer, int size) {
    /* Simple indexed access - ivopts may convert to pointer arithmetic */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(base + 0) with separate i increment */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses base[i] - base remains constant */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit pointer increment with stride */
float dot_product(const float *a, const float *b, int size) {
    float result = 0.0f;
    const float *pa = a;
    const float *pb = b;
    
    /* Explicit separate increment instructions */
    for (int i = 0; i < size; i++) {
        result += (*pa) * (*pb);  /* *(pa + 0) and *(pb + 0) */
        pa += 1;  /* Candidate increment for find_inc */
        pb += 1;  /* Candidate increment for find_inc */
    }
    return result;
}

/* Pattern 5: Struct access with pointer increment */
struct Point {
    int x;
    int y;
    int z;
};

int sum_points(const struct Point *points, int count) {
    int total = 0;
    const struct Point *ptr = points;
    
    /* Access struct fields - each field access uses ptr + offset */
    for (int i = 0; i < count; i++) {
        total += ptr->x + ptr->y;  /* Accesses at ptr + 0 and ptr + 4 */
        ptr++;  /* Increment by sizeof(struct Point) */
    }
    return total;
}

/* Pattern 6: Loop with multiple memory references to same base */
void copy_and_increment(int *dest, const int *src, int size) {
    const int *s = src;
    int *d = dest;
    
    for (int i = 0; i < size; i++) {
        *d = *s;  /* Two memory accesses with base+0 addressing */
        s++;      /* Increment for source pointer */
        d++;      /* Increment for destination pointer */
    }
}

/* Pattern 7: char pointer traversal (byte access) */
int count_chars(const char *str, char target) {
    int count = 0;
    const char *p = str;
    
    while (*p != '\0') {
        if (*p == target)  /* Access at p + 0 */
            count++;
        p++;  /* Byte increment */
    }
    return count;
}

/* Pattern 8: Loop with if-else creating multiple basic blocks */
void process_with_branch(int *data, int size, int threshold) {
    int *ptr = data;
    
    for (int i = 0; i < size; i++) {
        if (*ptr > threshold) {  /* Access at ptr + 0 */
            *ptr = threshold;
        } else {
            *ptr = *ptr * 2;  /* Another access at ptr + 0 */
        }
        ptr++;  /* Increment after the branch */
    }
}

int main() {
    /* Initialize test data */
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    float vec1[N], vec2[N];
    struct Point points[N];
    int dest[N];
    const char *test_str = "Hello, World!";
    int data[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = i * 2;
        vec1[i] = i * 1.0f;
        vec2[i] = i * 0.5f;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
        data[i] = i;
    }
    
    /* Execute all patterns to ensure code is used */
    int sum = sum_array(arr, N);
    printf("Sum of array: %d\n", sum);
    
    clear_buffer(buffer, N);
    printf("Buffer[0] after clear: %d\n", buffer[0]);
    
    fill_matrix(matrix);
    printf("Matrix[0][0]: %d\n", matrix[0][0]);
    
    float dot = dot_product(vec1, vec2, N);
    printf("Dot product: %f\n", dot);
    
    int point_sum = sum_points(points, N);
    printf("Sum of points: %d\n", point_sum);
    
    copy_and_increment(dest, arr, N);
    printf("Copied dest[0]: %d\n", dest[0]);
    
    int char_count = count_chars(test_str, 'l');
    printf("Count of 'l': %d\n", char_count);
    
    process_with_branch(data, N, 50);
    printf("Processed data[0]: %d\n", data[0]);
    
    return 0;
}
