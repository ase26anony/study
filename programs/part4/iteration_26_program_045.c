/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto_inc_dec optimization pass
 * Specifically targets the find_inc(true) path with reg1_val = 0
 */

#define SIZE 100
#define INNER_SIZE 50

/* Pattern 1: Simple pointer traversal with post-increment */
void pattern1_simple_pointer(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Classic *ptr++ pattern - access with offset 0, then increment */
    for (int i = 0; i < n; i++) {
        sum += *p;  /* Should become base + 0 access */
        p++;        /* Separate increment instruction */
    }
    
    /* Prevent dead code elimination */
    arr[0] = sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer induction */
void pattern2_indexed_access(char *buffer, int n) {
    /* Simple indexed access - ivopts may convert to pointer form */
    for (int i = 0; i < n; i++) {
        buffer[i] = (char)i;  /* May become *(base + 0) after optimization */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void pattern3_nested_loops(int matrix[][INNER_SIZE], int m, int n) {
    for (int j = 0; j < m; j++) {
        int *base = &matrix[j][0];  /* Base calculated in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < n; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Pointer with explicit stride */
void pattern4_explicit_stride(float *data, int n, int stride) {
    float *ptr = data;
    float total = 0.0f;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < n; i++) {
        total += *ptr;    /* base + 0 access */
        ptr += stride;    /* explicit increment by constant */
    }
    
    data[0] = total;
}

/* Pattern 5: Struct access to ensure proper scaling */
struct Point {
    int x;
    int y;
    int z;
};

void pattern5_struct_access(struct Point *points, int n) {
    struct Point *ptr = points;
    
    /* Struct access - increment by sizeof(struct Point) */
    for (int i = 0; i < n; i++) {
        ptr->x = i;      /* Should be *(ptr + 0) */
        ptr->y = i * 2;
        ptr->z = i * 3;
        ptr++;           /* Increment by 12 bytes (on typical systems) */
    }
}

/* Pattern 6: Multiple memory references in same loop */
void pattern6_multiple_refs(int *src, int *dst, int n) {
    int *s = src;
    int *d = dst;
    
    /* Two memory references, both with base + 0 pattern */
    for (int i = 0; i < n; i++) {
        *d = *s;    /* Both should be base + 0 */
        s++;
        d++;
    }
}

/* Pattern 7: Loop with if condition that doesn't break the pattern */
void pattern7_conditional(int *arr, int n, int threshold) {
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (*ptr > threshold) {  /* base + 0 access */
            *ptr = 0;            /* another base + 0 access */
        }
        ptr++;
    }
}

/* Pattern 8: Do-while loop variant */
void pattern8_dowhile(unsigned char *buf, int n) {
    unsigned char *p = buf;
    int count = n;
    
    do {
        *p = 0;        /* base + 0 access */
        p++;           /* increment */
    } while (--count > 0);
}

/* Main function to call all patterns */
int main() {
    /* Initialize test data */
    int array1[SIZE];
    char buffer[SIZE];
    int matrix[SIZE][INNER_SIZE];
    float float_data[SIZE];
    struct Point points[SIZE];
    int array2[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i;
        buffer[i] = 0;
        float_data[i] = i * 1.5f;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
        array2[i] = SIZE - i;
    }
    
    /* Call each pattern to ensure they're not optimized away */
    pattern1_simple_pointer(array1, SIZE);
    pattern2_indexed_access(buffer, SIZE);
    pattern3_nested_loops(matrix, SIZE/2, INNER_SIZE);
    pattern4_explicit_stride(float_data, SIZE, 1);
    pattern5_struct_access(points, SIZE);
    pattern6_multiple_refs(array1, array2, SIZE);
    pattern7_conditional(array1, SIZE, SIZE/2);
    pattern8_dowhile((unsigned char*)buffer, SIZE);
    
    /* Compute a checksum to prevent complete optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i] + (int)buffer[i];
    }
    
    return checksum == 0 ? 0 : 1;
}
