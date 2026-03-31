/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass where find_inc(true) is called with
 * a memory reference having reg1_val = 0 (base + 0 addressing).
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 16

/* Pattern 1: Simple pointer traversal with post-increment */
int sum_array(const int* arr, int size) {
    int sum = 0;
    const int* p = arr;
    
    /* Classic *ptr++ pattern - the dereference happens at *(ptr + 0)
     * and ptr is incremented separately */
    for (int i = 0; i < size; i++) {
        sum += *p;      /* Should become *(p + 0) after optimization */
        p++;            /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void clear_buffer(int* buffer, int size) {
    /* Simple indexed access - ivopts may convert to pointer form */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* May become *(base + 0) with separate i increment */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void fill_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int* base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit pointer arithmetic with stride */
float dot_product(const float* a, const float* b, int size) {
    float result = 0.0f;
    const float* pa = a;
    const float* pb = b;
    
    /* Explicit separate access and increment */
    for (int i = 0; i < size; i++) {
        result += (*pa) * (*pb);  /* *(pa + 0) and *(pb + 0) */
        pa += 1;                  /* Separate increment */
        pb += 1;                  /* Separate increment */
    }
    return result;
}

/* Pattern 5: Struct access with pointer traversal */
struct Point {
    int x;
    int y;
    int z;
};

int sum_points(const struct Point* points, int count) {
    int total = 0;
    const struct Point* ptr = points;
    
    /* Struct access - larger stride may still trigger the pattern */
    for (int i = 0; i < count; i++) {
        total += ptr->x + ptr->y;  /* Access at ptr + 0 */
        ptr++;                     /* Increment by sizeof(struct Point) */
    }
    return total;
}

/* Pattern 6: char array processing - small stride */
int count_chars(const char* str, char target) {
    int count = 0;
    const char* p = str;
    
    while (*p != '\0') {
        if (*p == target)  /* *(p + 0) */
            count++;
        p++;                /* Separate increment by 1 */
    }
    return count;
}

/* Pattern 7: Loop with multiple memory references */
void copy_and_scale(int* dest, const int* src, int size, int factor) {
    const int* s = src;
    int* d = dest;
    
    for (int i = 0; i < size; i++) {
        *d = (*s) * factor;  /* Two memory references: *(s + 0) and *(d + 0) */
        s++;                 /* Separate increment */
        d++;                 /* Separate increment */
    }
}

/* Pattern 8: Loop with if-else but consistent pointer access */
void process_conditional(int* data, int size, int threshold) {
    int* ptr = data;
    
    for (int i = 0; i < size; i++) {
        if (*ptr > threshold) {  /* *(ptr + 0) */
            *ptr = threshold;
        } else {
            *ptr = *ptr * 2;     /* Still *(ptr + 0) */
        }
        ptr++;                   /* Separate increment */
    }
}

int main() {
    /* Initialize test data */
    int arr[N];
    int buffer[N];
    int matrix[M][N];
    float vec1[N], vec2[N];
    struct Point points[N];
    char str[] = "test string for auto-inc-dec optimization";
    int src[N], dest[N];
    int data[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        buffer[i] = i * 2;
        vec1[i] = i * 1.5f;
        vec2[i] = i * 0.5f;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
        src[i] = i;
        data[i] = i % 10;
    }
    
    /* Execute all patterns to ensure code is generated */
    int sum = sum_array(arr, N);
    printf("Sum: %d\n", sum);
    
    clear_buffer(buffer, N);
    printf("Buffer[0]: %d\n", buffer[0]);
    
    fill_matrix(matrix);
    printf("Matrix[0][0]: %d\n", matrix[0][0]);
    
    float dot = dot_product(vec1, vec2, N);
    printf("Dot product: %f\n", dot);
    
    int point_sum = sum_points(points, N);
    printf("Point sum: %d\n", point_sum);
    
    int char_count = count_chars(str, 't');
    printf("'t' count: %d\n", char_count);
    
    copy_and_scale(dest, src, N, 2);
    printf("Dest[0]: %d\n", dest[0]);
    
    process_conditional(data, N, 5);
    printf("Data[0]: %d\n", data[0]);
    
    return 0;
}
