/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Opaque external functions to create aliasing concerns */
extern void use_int_ptr(int *p) __attribute__((noinline, noipa));
extern void use_char_ptr(char *p) __attribute__((noinline, noipa));
extern void use_void_ptr(void *p) __attribute__((noinline, noipa));

/* Prevent compiler from optimizing away pointer operations */
#define KEEP_ALIVE(ptr) asm volatile("" : : "r"(ptr) : "memory")

/* Test 1: Integer array summation with post-increment pointer */
int test_int_array_sum(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Create register pressure with many live variables */
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0;
    
    while (p < end) {
        /* Post-increment access - target pattern for find_inc() */
        sum += *p++;
        
        /* Use the live variables to increase register pressure */
        r0 += sum; r1 += r0; r2 += r1; r3 += r2;
        r4 += r3; r5 += r4; r6 += r5; r7 += r6;
        
        /* Prevent optimization of pointer */
        KEEP_ALIVE(p);
    }
    
    /* Mix results to prevent elimination */
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    
    /* Opaque function call creates aliasing */
    use_int_ptr(arr);
    
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test_char_array_copy(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src;
    int i = 0;
    
    /* Register pressure variables */
    char c0 = 0, c1 = 0, c2 = 0, c3 = 0;
    int t0 = 0, t1 = 0, t2 = 0, t3 = 0;
    
    /* Copy with post-increment - classic pattern for auto-inc-dec */
    do {
        *d++ = *s++;
        i++;
        
        /* Use variables to keep them live */
        c0 = *s; c1 = c0 + 1; c2 = c1 + 1; c3 = c2 + 1;
        t0 += c0; t1 += c1; t2 += c2; t3 += c3;
        
        /* Prevent optimization */
        KEEP_ALIVE(d);
        KEEP_ALIVE(s);
    } while (i < n);
    
    /* Opaque calls */
    use_char_ptr(dst);
    use_char_ptr((char *)src);
    
    global_checksum += t0 + t1 + t2 + t3;
}

/* Simple struct for testing */
struct Point {
    int x;
    int y;
    int z;
};

/* Test 3: Struct array traversal with post-increment */
int test_struct_array(struct Point *points, int n) {
    int total = 0;
    struct Point *p = points;
    struct Point *end = points + n;
    
    /* High register pressure */
    int a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0;
    struct Point tmp;
    
    while (p < end) {
        /* Access struct member via post-increment pointer */
        total += p->x + p->y;
        
        /* Post-increment the pointer */
        struct Point *old_p = p++;
        
        /* Complex operations to prevent optimization */
        tmp.x = old_p->x;
        tmp.y = old_p->y;
        tmp.z = old_p->z;
        
        a0 += tmp.x; a1 += tmp.y; a2 += tmp.z;
        a3 += a0; a4 += a1; a5 += a2;
        
        /* Force pointer to stay in register */
        KEEP_ALIVE(p);
        KEEP_ALIVE(old_p);
    }
    
    total += a0 + a1 + a2 + a3 + a4 + a5;
    use_void_ptr(points);
    
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int **matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l, m, n;
    int *row_ptr;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        
        /* Inner loop with pointer arithmetic */
        for (j = 0; j < cols; j++) {
            /* Combined form that may decompose to base+offset */
            sum += *(row_ptr + j);
            
            /* Alternative: post-increment in separate expression */
            int *tmp_ptr = row_ptr + j;
            sum += *tmp_ptr;
            
            /* More register pressure */
            k = i * j;
            l = k + sum;
            m = l * 2;
            n = m / 3;
            
            sum += n;
        }
        
        /* Pointer update that might trigger find_inc */
        int *p = matrix[i];
        for (j = 0; j < cols; j++) {
            /* Post-increment access in loop body */
            int val = *p++;
            sum += val;
            
            /* Prevent optimization */
            KEEP_ALIVE(p);
        }
    }
    
    return sum;
}

/* Test 5: Mixed pointer types and stride access */
int test_mixed_pointers(char *data, int size, int stride) {
    int sum = 0;
    char *p = data;
    int *ip;
    
    /* Multiple pointer variables alive simultaneously */
    char *q = p + size/2;
    int *iq = (int *)q;
    
    for (int i = 0; i < size/4; i++) {
        /* Access with pointer += stride pattern */
        ip = (int *)p;
        sum += *ip;
        
        /* Post-increment with stride */
        p += stride;
        
        /* Another pointer with different type */
        sum += *iq++;
        
        /* Register pressure */
        int t0 = *ip;
        int t1 = *iq;
        int t2 = t0 + t1;
        int t3 = t2 * 2;
        
        sum += t3;
        
        /* Keep pointers alive */
        KEEP_ALIVE(p);
        KEEP_ALIVE(ip);
        KEEP_ALIVE(iq);
    }
    
    return sum;
}

/* Main function with command-line control */
int main(int argc, char **argv) {
    /* Initialize test data */
    const int INT_ARRAY_SIZE = 1024;
    const int CHAR_ARRAY_SIZE = 512;
    const int STRUCT_ARRAY_SIZE = 256;
    const int MATRIX_ROWS = 32;
    const int MATRIX_COLS = 32;
    
    /* Allocate and initialize arrays */
    int *int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char *char_array_src = malloc(CHAR_ARRAY_SIZE);
    char *char_array_dst = malloc(CHAR_ARRAY_SIZE);
    struct Point *struct_array = malloc(STRUCT_ARRAY_SIZE * sizeof(struct Point));
    int **matrix = malloc(MATRIX_ROWS * sizeof(int *));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 97;
    }
    
    for (int i = 0; i < CHAR_ARRAY_SIZE; i++) {
        char_array_src[i] = (i % 26) + 'a';
    }
    
    for (int i = 0; i < STRUCT_ARRAY_SIZE; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 2;
        struct_array[i].z = i * 3;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = (i * MATRIX_COLS + j) % 127;
        }
    }
    
    /* Use command-line arguments to control which tests run */
    int test_mask = 0x1F; /* Run all tests by default */
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    }
    
    int total_sum = 0;
    
    if (test_mask & 0x01) {
        total_sum += test_int_array_sum(int_array, INT_ARRAY_SIZE);
    }
    
    if (test_mask & 0x02) {
        test_char_array_copy(char_array_dst, char_array_src, CHAR_ARRAY_SIZE);
        total_sum += char_array_dst[0] + char_array_dst[CHAR_ARRAY_SIZE-1];
    }
    
    if (test_mask & 0x04) {
        total_sum += test_struct_array(struct_array, STRUCT_ARRAY_SIZE);
    }
    
    if (test_mask & 0x08) {
        total_sum += test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
    }
    
    if (test_mask & 0x10) {
        total_sum += test_mixed_pointers(char_array_src, CHAR_ARRAY_SIZE, 4);
    }
    
    /* Update global volatile to ensure side effects */
    global_checksum += total_sum;
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(char_array_src);
    free(char_array_dst);
    free(struct_array);
    
    printf("Checksum: %d\n", total_sum);
    return 0;
}

/* Dummy implementations of opaque functions */
void use_int_ptr(int *p) {
    /* Empty but prevents optimization */
    (void)p;
}

void use_char_ptr(char *p) {
    (void)p;
}

void use_void_ptr(void *p) {
    (void)p;
}
