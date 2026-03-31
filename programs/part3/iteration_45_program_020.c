/* auto_inc_dec_test.c - Test program for auto-increment/decrement optimization */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void barrier(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct to create complex memory accesses */
struct Data {
    int value;
    char tag;
    int payload[2];
};

/* Prevent inlining to preserve patterns */
__attribute__((noinline)) 
int test_post_increment_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* Pattern: *p++ - should generate (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use temporaries to increase register pressure */
        temp1 = sum * 2;
        temp2 = temp1 + 1;
        temp3 = temp2 * 3;
        temp4 = temp3 - sum;
        temp5 = temp4 ^ temp1;
        temp6 = temp5 | temp2;
        temp7 = temp6 & temp3;
        temp8 = temp7 + temp4;
        
        /* Use asm to prevent optimization */
        asm volatile("" : : "r"(p), "r"(sum), "r"(temp1), "r"(temp2),
                       "r"(temp3), "r"(temp4), "r"(temp5), "r"(temp6));
    }
    
    /* Use all temporaries to prevent elimination */
    sum += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
    
    /* Opaque function call */
    use_int(sum);
    
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
    
    /* Classic *dst++ = *src++ pattern */
    while (s < end) {
        /* Should generate two mem accesses with post-increment */
        *d++ = *s++;
        
        /* Additional operations for register pressure */
        c1 = *(s - 1);
        c2 = c1 + 1;
        c3 = c2 * 2;
        c4 = c3 ^ c1;
        
        i1 += c1;
        i2 += c2;
        i3 += c3;
        i4 += c4;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(d), "r"(s), "r"(c1), "r"(i1));
    }
    
    /* Use variables */
    use_char(c1);
    use_int(i1 + i2 + i3 + i4);
}

__attribute__((noinline))
int test_struct_array_traversal(struct Data* arr, int n) {
    int total = 0;
    struct Data* p = arr;
    
    /* Many local variables for register pressure */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    int v5 = 0, v6 = 0, v7 = 0, v8 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->value;
        
        /* Complex expression with pointer arithmetic */
        int* payload_ptr = p->payload;
        v1 += *payload_ptr++;
        v2 += *payload_ptr;
        
        /* Post-increment the struct pointer */
        p++;
        
        /* More register pressure calculations */
        v3 = total * v1;
        v4 = v2 ^ v3;
        v5 = v4 + v1;
        v6 = v5 - v2;
        v7 = v6 * 3;
        v8 = v7 / 2;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total), "r"(v1), "r"(v2),
                       "r"(v3), "r"(v4), "r"(v5), "r"(v6));
    }
    
    total += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    use_int(total);
    return total;
}

__attribute__((noinline))
int test_nested_loops_with_index(int* matrix, int rows, int cols) {
    int sum = 0;
    
    /* High register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int r5 = 0, r6 = 0, r7 = 0, r8 = 0;
    
    for (int i = 0; i < rows; i++) {
        int* row_ptr = matrix + i * cols;
        int* row_end = row_ptr + cols;
        
        /* Inner loop with post-increment */
        while (row_ptr < row_end) {
            /* Pattern: array access with post-increment */
            sum += *row_ptr++;
            
            /* Index calculations for register pressure */
            r1 = sum * i;
            r2 = r1 + (int)(row_ptr - matrix);
            r3 = r2 ^ sum;
            r4 = r3 * 2;
            r5 = r4 - r1;
            r6 = r5 | r2;
            r7 = r6 & r3;
            r8 = r7 + r4;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(row_ptr), "r"(sum), "r"(r1), "r"(r2));
        }
        
        /* Use all register pressure variables */
        sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    }
    
    use_int(sum);
    return sum;
}

__attribute__((noinline))
int test_mixed_pointer_arithmetic(int* arr, int n, int stride) {
    int result = 0;
    int* p = arr;
    int* end = arr + n * stride;
    
    /* Combined forms like *(p += stride) */
    while (p < end) {
        /* This may decompose into base+offset forms */
        result += *p;
        p += stride;
        
        /* Additional pointer arithmetic */
        int* q = p - 1;
        result += *q;
        
        /* More complex pattern */
        int* r = p;
        if (r < end) {
            result += *(r++);
            result += *(r);
        }
        
        /* Register pressure */
        asm volatile("" : : "r"(p), "r"(q), "r"(r), "r"(result));
    }
    
    use_int(result);
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
    const int INT_ARRAY_SIZE = 1000;
    const int CHAR_ARRAY_SIZE = 500;
    const int STRUCT_COUNT = 200;
    const int MATRIX_ROWS = 50;
    const int MATRIX_COLS = 20;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(INT_ARRAY_SIZE * sizeof(int));
    char* char_array1 = (char*)malloc(CHAR_ARRAY_SIZE);
    char* char_array2 = (char*)malloc(CHAR_ARRAY_SIZE);
    struct Data* struct_array = (struct Data*)malloc(STRUCT_COUNT * sizeof(struct Data));
    int* matrix = (int*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) ^ (argc + i);
    }
    
    for (int i = 0; i < CHAR_ARRAY_SIZE; i++) {
        char_array1[i] = (char)((i * 7) & 0xFF);
        char_array2[i] = 0;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = i * 2 + argc;
        struct_array[i].tag = (char)i;
        struct_array[i].payload[0] = i * 3;
        struct_array[i].payload[1] = i * 5;
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = (i * 11) ^ argc;
    }
    
    /* Run tests based on command line arguments */
    int test_to_run = (argc > 1) ? atoi(argv[1]) : 0;
    
    barrier();
    
    switch (test_to_run % 5) {
        case 0:
            global_checksum += test_post_increment_sum(int_array, INT_ARRAY_SIZE);
            break;
        case 1:
            test_post_increment_copy(char_array2, char_array1, CHAR_ARRAY_SIZE);
            global_checksum += char_array2[0] + char_array2[CHAR_ARRAY_SIZE-1];
            break;
        case 2:
            global_checksum += test_struct_array_traversal(struct_array, STRUCT_COUNT);
            break;
        case 3:
            global_checksum += test_nested_loops_with_index(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            global_checksum += test_mixed_pointer_arithmetic(int_array, INT_ARRAY_SIZE/10, 10);
            break;
    }
    
    /* Clean up */
    free(int_array);
    free(char_array1);
    free(char_array2);
    free(struct_array);
    free(matrix);
    
    /* Print result to prevent elimination */
    printf("Checksum: %d\n", global_checksum);
    
    return global_checksum != 0 ? 0 : 1;
}
