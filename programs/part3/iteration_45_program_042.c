/* test_auto_inc_dec.c - Program to trigger specific auto-inc-dec optimization paths */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void barrier(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_sum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int count;
    float data;
};

/* Test 1: Integer array summation with post-increment pointer */
int test_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0;
    
    while (p < end) {
        /* Post-increment access - target pattern for find_inc() */
        sum += *p++;
        
        /* Use all live variables to increase register pressure */
        r0 += sum & 1;
        r1 += sum & 2;
        r2 += sum & 4;
        r3 += sum & 8;
        r4 += sum & 16;
        r5 += sum & 32;
        r6 += sum & 64;
        r7 += sum & 128;
        
        /* Prevent optimization with asm barrier */
        asm volatile("" : : "r"(p), "r"(sum) : "memory");
    }
    
    /* Combine all register pressure variables */
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    
    /* Opaque use to prevent elimination */
    use_int(sum);
    use_ptr(p);
    
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test_string_copy(char* dst, const char* src, size_t len) {
    char* d = dst;
    const char* s = src;
    const char* end = src + len;
    
    /* Register pressure variables */
    char c0 = 0, c1 = 0, c2 = 0, c3 = 0;
    int checksum = 0;
    
    while (s < end) {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Additional operations to create complex RTL */
        c0 = *s;
        c1 = *d;
        checksum += c0 + c1;
        
        /* Use asm to make pointers appear live */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
    }
    
    /* Force use of variables */
    use_char(c0);
    use_char(c1);
    use_int(checksum);
    
    /* Opaque barrier */
    barrier();
}

/* Test 3: Struct array traversal with post-increment */
int test_struct_array(struct Data* arr, int n) {
    int total = 0;
    struct Data* p = arr;
    struct Data* end = arr + n;
    
    /* High register pressure */
    int acc0 = 0, acc1 = 0, acc2 = 0, acc3 = 0;
    float f0 = 0.0f, f1 = 0.0f;
    
    for (; p < end; p++) {
        /* Access struct member with pointer that will be incremented */
        total += p->value;
        
        /* More operations to create addressing modes */
        acc0 += p->count;
        f0 += p->data;
        
        /* Mixed pointer arithmetic */
        struct Data* q = p + 1;
        if (q < end) {
            acc1 += q->value;
        }
        
        /* Complex addressing pattern */
        total += (p->tag == 'A') ? 10 : 0;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total) : "memory");
    }
    
    /* Combine all accumulators */
    total += acc0 + acc1 + (int)f0;
    
    use_int(total);
    use_ptr(p);
    
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i = 0, j = 0, k = 0, l = 0;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        int* end = row + cols;
        
        /* Inner loop with post-increment pointer */
        while (p < end) {
            /* Target pattern: *(p + 0) with post-increment */
            sum += *p++;
            
            /* Additional index manipulation */
            j = (int)(p - row);
            k = cols - j;
            l = i * j;
            
            /* Use all variables */
            sum += k + l;
            
            /* Memory barrier */
            asm volatile("" : : "r"(p), "r"(j) : "memory");
        }
        
        /* Complex pointer expression that may decompose to base+0 */
        int* q = matrix[i] + (cols / 2);
        if (q < row + cols) {
            sum += *q;
        }
    }
    
    use_int(sum);
    return sum;
}

/* Test 5: Mixed pointer arithmetic with stride */
int test_mixed_arithmetic(int* arr, int n, int stride) {
    int result = 0;
    int* p = arr;
    int* limit = arr + n * stride;
    
    /* Multiple stride pointers */
    int* p1 = arr;
    int* p2 = arr + stride;
    int* p3 = arr + 2 * stride;
    
    while (p < limit) {
        /* Combined form: *(p += stride) */
        result += *p;
        p += stride;
        
        /* Multiple parallel accesses */
        if (p1 < limit) result += *p1++;
        if (p2 < limit) result += *p2++;
        if (p3 < limit) result += *p3++;
        
        /* Register pressure */
        int t0 = result * 2;
        int t1 = result / 3;
        int t2 = result ^ 0x55;
        
        result += t0 + t1 + t2;
        
        /* Force address computation */
        asm volatile("" : : "r"(p), "r"(p1), "r"(p2), "r"(p3) : "memory");
    }
    
    return result;
}

/* Test 6: Do-while loop with post-decrement */
int test_post_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* Register pressure */
    int a = 0, b = 0, c = 0, d = 0;
    
    do {
        /* Post-decrement access */
        sum += *p--;
        
        /* Use multiple variables */
        a = sum & 0xFF;
        b = (sum >> 8) & 0xFF;
        c = (sum >> 16) & 0xFF;
        d = (sum >> 24) & 0xFF;
        
        sum += a + b + c + d;
        
        /* Memory barrier */
        asm volatile("" : : "r"(p) : "memory");
    } while (p >= arr);
    
    return sum;
}

/* Dummy implementations of opaque functions */
void use_int(int x) {
    global_sum += x;
}

void use_ptr(void* p) {
    global_sum += (int)(intptr_t)p;
}

void use_char(char c) {
    global_sum += c;
}

void barrier(void) {
    asm volatile("" : : : "memory");
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1000;
    const int MATRIX_ROWS = 10;
    const int MATRIX_COLS = 10;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = malloc(ARRAY_SIZE * sizeof(struct Data));
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'A' + (i % 26);
        struct_array[i].value = i * 2;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].count = i;
        struct_array[i].data = i * 0.5f;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    /* Select test based on command line argument */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    switch (test_num) {
        case 0:
            result = test_int_array_sum(int_array, ARRAY_SIZE);
            break;
        case 1:
            test_string_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result = global_sum;
            break;
        case 2:
            result = test_struct_array(struct_array, ARRAY_SIZE);
            break;
        case 3:
            result = test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            result = test_mixed_arithmetic(int_array, ARRAY_SIZE/10, 10);
            break;
        case 5:
            result = test_post_decrement(int_array, ARRAY_SIZE);
            break;
        default:
            /* Run all tests */
            result = test_int_array_sum(int_array, ARRAY_SIZE);
            test_string_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result += test_struct_array(struct_array, ARRAY_SIZE);
            result += test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test_mixed_arithmetic(int_array, ARRAY_SIZE/10, 10);
            result += test_post_decrement(int_array, ARRAY_SIZE);
            break;
    }
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(char_array);
    free(struct_array);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
