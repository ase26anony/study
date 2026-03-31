/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto_inc_dec optimization pass
 * Specifically targets the find_inc(true) path with reg1_val = 0
 */

#define SIZE 100
#define INNER_SIZE 50

/* Simple pointer traversal with post-increment */
void pointer_traversal(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Pattern 1: Classic *ptr++ which often becomes *(ptr + 0) in RTL */
    for (int i = 0; i < n; i++) {
        sum += *p;  /* Memory access with base + 0 */
        p++;        /* Separate increment instruction */
    }
}

/* Indexed access where ivopts creates pointer with zero offset */
void indexed_access(int *buffer, int n) {
    /* Pattern 2: Array indexing with post-increment of index */
    for (int i = 0; i < n; i++) {
        buffer[i] = i * 2;  /* May become *(base + 0) after optimization */
        /* i++ happens here, separate from memory access */
    }
}

/* Nested loops with invariant base in inner loop */
void nested_loops(int matrix[][INNER_SIZE], int m, int n) {
    /* Pattern 3: Base pointer calculated in outer loop, constant in inner */
    for (int j = 0; j < m; j++) {
        int *base = &matrix[j][0];  /* Base calculated once per outer iteration */
        
        for (int i = 0; i < n; i++) {
            base[i] = i + j;  /* Access relative to base with zero offset */
            /* i++ is separate increment */
        }
    }
}

/* Explicit pointer arithmetic with stride */
void strided_access(float *data, int n, int stride) {
    float *ptr = data;
    float total = 0.0f;
    
    /* Pattern 4: Explicit pointer increment separate from access */
    for (int i = 0; i < n; i++) {
        total += *ptr;    /* *(ptr + 0) */
        ptr += stride;    /* Explicit increment, find_inc should find this */
    }
}

/* Struct access to test with different data types */
struct Point {
    int x;
    int y;
    int z;
};

void struct_traversal(struct Point *points, int n) {
    struct Point *p = points;
    
    /* Pattern 5: Struct pointer traversal */
    for (int i = 0; i < n; i++) {
        p->x = i;        /* Memory access: base + 0 */
        p->y = i * 2;
        p->z = i * 3;
        p++;             /* Separate increment by sizeof(struct Point) */
    }
}

/* Mixed patterns to increase coverage probability */
void mixed_patterns(int *arr1, int *arr2, int n) {
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; i++) {
        /* Multiple memory accesses with same base */
        int val = *p1;          /* First access: *(p1 + 0) */
        *p2 = val * 2;          /* Second access: *(p2 + 0) */
        
        /* Increments happen separately */
        p1++;
        p2++;
    }
}

/* Main function with compile-time constants for better optimization */
int main() {
    int array1[SIZE];
    int array2[SIZE];
    float float_array[SIZE];
    int matrix[SIZE][INNER_SIZE];
    struct Point points[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i;
        array2[i] = 0;
        float_array[i] = i * 1.5f;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
    }
    
    /* Call all test functions to ensure code generation */
    pointer_traversal(array1, SIZE);
    indexed_access(array2, SIZE);
    nested_loops(matrix, SIZE/2, INNER_SIZE);
    strided_access(float_array, SIZE, 1);
    struct_traversal(points, SIZE);
    mixed_patterns(array1, array2, SIZE);
    
    return 0;
}
