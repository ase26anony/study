/* test-auto-inc-dec.c
 * Program designed to trigger auto-increment/decrement optimization patterns
 * Specifically targets find_inc() logic with mem_loc = address_of_x, reg0 = XEXP(x,0), reg1_is_const=true, reg1_val=0
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void barrier(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int id;
};

/* Prevent inlining to preserve patterns */
__attribute__((noinline))
int test_post_increment_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 = sum * 2;
        r2 = r1 + 3;
        r3 = r2 - r1;
        r4 = r3 * r2;
        r5 = r4 / (r1 + 1);
        r6 = r5 ^ r4;
        r7 = r6 & 0xFF;
        r8 = r7 | 0x80;
        
        /* Make pointer appear used */
        asm volatile("" : : "r"(p));
    }
    
    /* Use all pressure variables */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    
    /* Opaque use to prevent elimination */
    use_ptr(p);
    return sum;
}

__attribute__((noinline))
void test_post_increment_copy(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    const char* end = src + n;
    
    /* Register pressure variables */
    char c1, c2, c3, c4;
    int i1 = 0, i2 = 0, i3 = 0, i4 = 0;
    
    while (s < end) {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Additional operations to prevent over-optimization */
        c1 = *(s - 1);
        c2 = *(d - 1);
        c3 = c1 ^ c2;
        c4 = c3 & 0x7F;
        
        i1 += c1;
        i2 += c2;
        i3 += c3;
        i4 += c4;
        
        /* Force pointer usage */
        asm volatile("" : : "r"(s), "r"(d));
    }
    
    /* Use variables */
    global_checksum += i1 + i2 + i3 + i4;
    
    /* Opaque calls */
    use_ptr(d);
    use_ptr(s);
}

__attribute__((noinline))
int test_struct_traversal(struct Data* data, int count) {
    int total = 0;
    struct Data* p = data;
    struct Data* end = data + count;
    
    /* Heavy register pressure */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0, v6 = 0, v7 = 0, v8 = 0;
    char t1, t2, t3, t4;
    
    for (; p < end; p++) {
        /* Access struct member with pointer that gets incremented */
        total += p->value;
        
        /* More operations for pressure */
        t1 = p->tag;
        t2 = t1 + 1;
        t3 = t2 * 2;
        t4 = t3 & 0xF;
        
        v1 += p->id;
        v2 += t1;
        v3 += t2;
        v4 += t3;
        v5 += t4;
        v6 = v1 * v2;
        v7 = v3 ^ v4;
        v8 = v5 + v6 + v7;
        
        /* Force pointer to be live */
        asm volatile("" : : "r"(p));
    }
    
    total += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    use_ptr(p);
    return total;
}

__attribute__((noinline))
int test_nested_loops(int* matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for pressure */
    int i, j, k, l, m, n, o, p;
    i = j = k = l = m = n = o = p = 0;
    
    for (i = 0; i < rows; i++) {
        int* row_ptr = matrix + i * cols;
        int* row_end = row_ptr + cols;
        
        /* Inner loop with post-increment */
        for (j = 0; row_ptr < row_end; row_ptr++) {
            /* Post-increment access in nested loop */
            sum += *row_ptr;
            
            /* Complex indexing calculations */
            k = i * cols + j;
            l = k * 2;
            m = l - j;
            n = m ^ k;
            o = n & 0xFF;
            p = o | 0x80;
            
            j++;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(row_ptr), "r"(row_end));
        }
        
        sum += k + l + m + n + o + p;
    }
    
    return sum;
}

__attribute__((noinline))
int test_mixed_operations(int* arr, int n, int stride) {
    int result = 0;
    int* p = arr;
    int* end = arr + n * stride;
    
    /* Combined form: *(p += stride) which may decompose to base+0 */
    for (; p < end; p += stride) {
        result += *p;
        
        /* Additional pointer arithmetic that might create (plus (reg) (const_int 0)) */
        int* q = p + 1;
        int* r = q - 1;
        
        /* Force different addressing modes */
        result += *(r + 0);  /* Explicit plus 0 */
        
        /* Use all pointers */
        asm volatile("" : : "r"(p), "r"(q), "r"(r));
        
        /* Call opaque function */
        use_ptr(p);
    }
    
    return result;
}

/* Opaque function implementations */
void use_int(int x) {
    global_checksum ^= x;
}

void use_ptr(void* p) {
    global_checksum += (int)((size_t)p & 0xFFFF);
}

void use_char(char c) {
    global_checksum += c;
}

void barrier(void) {
    asm volatile("" : : : "memory");
}

int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 32;
    const int MATRIX_COLS = 32;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_array1 = (char*)malloc(ARRAY_SIZE * sizeof(char));
    char* char_array2 = (char*)malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = (struct Data*)malloc(ARRAY_SIZE * sizeof(struct Data));
    int* matrix = (int*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 97;
        char_array1[i] = 'A' + (i % 26);
        struct_array[i].value = i * 2;
        struct_array[i].tag = 'a' + (i % 26);
        struct_array[i].id = i;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = (i * 7) % 113;
    }
    
    /* Use command line to select tests, preventing constant folding */
    int test_mask = 0;
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    } else {
        test_mask = 0x1F; /* Run all tests */
    }
    
    int total_result = 0;
    
    /* Run selected tests */
    if (test_mask & 0x01) {
        total_result += test_post_increment_sum(int_array, ARRAY_SIZE);
    }
    
    if (test_mask & 0x02) {
        test_post_increment_copy(char_array2, char_array1, ARRAY_SIZE);
        total_result += global_checksum;
    }
    
    if (test_mask & 0x04) {
        total_result += test_struct_traversal(struct_array, ARRAY_SIZE / 4);
    }
    
    if (test_mask & 0x08) {
        total_result += test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
    }
    
    if (test_mask & 0x10) {
        total_result += test_mixed_operations(int_array, ARRAY_SIZE / 8, 4);
    }
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", total_result);
    
    /* Cleanup */
    free(int_array);
    free(char_array1);
    free(char_array2);
    free(struct_array);
    free(matrix);
    
    return total_result != 0 ? 0 : 1;
}
