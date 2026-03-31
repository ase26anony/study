/* test_auto_inc_dec.c - Program to trigger auto-increment/decrement optimization patterns */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void use_long(long) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    long timestamp;
    float weight;
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_sum_int_array(const int* arr, size_t n) {
    int sum = 0;
    const int* p = arr;
    const int* end = arr + n;
    
    /* Create register pressure with many local variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Use asm to prevent optimization of pointer */
    asm volatile("" : : "r"(p), "r"(end) : "memory");
    
    while (p < end) {
        /* Post-increment access - target pattern for find_inc() */
        sum += *p++;
        
        /* Use temporary variables to create register pressure */
        temp1 = sum * 2;
        temp2 = temp1 + 1;
        temp3 = temp2 * 3;
        temp4 = temp3 - sum;
        
        acc1 += temp1;
        acc2 += temp2;
        acc3 += temp3;
        acc4 += temp4;
        
        /* Prevent loop unrolling from eliminating the pattern */
        if ((p - arr) % 8 == 0) {
            use_int(acc1);
            asm volatile("" : : "r"(p) : "memory");
        }
    }
    
    /* Mix all accumulators to ensure they're used */
    sum += acc1 + acc2 + acc3 + acc4;
    global_checksum ^= sum;
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_copy_string(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    const char* end = src + n;
    
    /* Register pressure variables */
    int char_sum = 0;
    int shift1 = 0, shift2 = 0, shift3 = 0;
    
    /* Force pointer values to be live */
    use_ptr(d);
    use_ptr(s);
    
    while (s < end) {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Additional operations on the copied character */
        char last_char = *(d - 1);
        char_sum += last_char;
        shift1 = (char_sum << 1) | (char_sum >> 7);
        shift2 = (char_sum << 2) | (char_sum >> 6);
        shift3 = (char_sum << 3) | (char_sum >> 5);
        
        /* Periodically use pointers to keep them live */
        if ((s - src) % 16 == 0) {
            asm volatile("" : : "r"(d), "r"(s) : "memory");
            use_int(shift1 + shift2 + shift3);
        }
    }
    
    *d = '\0';
    global_checksum += char_sum;
}

/* Test 3: Struct array traversal with post-increment */
long test3_process_structs(struct Data* data, size_t count) {
    struct Data* p = data;
    struct Data* end = data + count;
    long total = 0;
    
    /* Many local variables for register pressure */
    long running[8] = {0};
    int idx = 0;
    
    /* Make pointer appear used in complex ways */
    asm volatile("" : : "r"(p), "r"(end) : "memory");
    
    for (; p < end; p++) {  /* Post-increment in loop update */
        /* Access struct member - creates base+0 addressing */
        total += p->value;
        
        /* Additional computations to use registers */
        running[idx] += p->timestamp;
        idx = (idx + 1) & 7;
        
        /* Complex expression that uses the pointer */
        long temp = (long)(p->weight * 100.0f);
        running[idx] ^= temp;
        
        /* Force pointer to stay in register */
        if ((p - data) % 4 == 0) {
            use_ptr(p);
            asm volatile("" : : "r"(p) : "memory");
        }
    }
    
    /* Combine all running totals */
    for (int i = 0; i < 8; i++) {
        total += running[i];
    }
    
    global_checksum += (int)(total & 0xFFFFFFFF);
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_matrix_sum(int matrix[][16], size_t rows) {
    int total = 0;
    
    /* Multiple accumulators for register pressure */
    int row_sums[4] = {0};
    int col_acc[4] = {0};
    
    for (size_t i = 0; i < rows; i++) {
        int* row_ptr = matrix[i];
        int* row_end = row_ptr + 16;
        
        /* Inner loop with post-increment pointer */
        while (row_ptr < row_end) {
            /* Post-increment access in loop body */
            int val = *row_ptr++;
            total += val;
            
            /* Multiple parallel accumulators */
            row_sums[i % 4] += val;
            col_acc[(row_ptr - matrix[i] - 1) % 4] += val;
            
            /* Complex addressing pattern */
            if ((row_ptr - matrix[i]) % 8 == 0) {
                /* Force base+0 addressing mode consideration */
                asm volatile("" : : "r"(row_ptr), "r"(row_end) : "memory");
                use_int(val * 2);
            }
        }
        
        /* Inter-row computations */
        for (int j = 0; j < 4; j++) {
            col_acc[j] ^= row_sums[j];
            row_sums[j] = 0;
        }
    }
    
    /* Final combination */
    for (int j = 0; j < 4; j++) {
        total += col_acc[j];
    }
    
    global_checksum ^= total;
    return total;
}

/* Test 5: Mixed pointer types and stride access */
void test5_mixed_pointers(char* data, size_t size, int stride) {
    char* p = data;
    char* end = data + size;
    int sum = 0;
    
    /* Multiple pointer variables */
    char* alt_ptr = data + (size / 2);
    int* int_view = (int*)data;
    
    /* Create aliasing concerns */
    use_ptr(p);
    use_ptr(alt_ptr);
    use_ptr((void*)int_view);
    
    /* Loop with stride-based access */
    do {
        /* Post-increment with stride */
        char c = *p;
        p += stride;
        
        sum += c;
        
        /* Access through different pointer type */
        if (p < end - sizeof(int)) {
            int val = *int_view++;
            sum += val & 0xFF;
            
            /* Force register allocation of both pointers */
            asm volatile("" : : "r"(p), "r"(int_view) : "memory");
        }
        
        /* Additional computation with many temporaries */
        int t1 = sum * 3;
        int t2 = sum / 2;
        int t3 = sum << 1;
        int t4 = sum >> 1;
        
        sum = t1 ^ t2 ^ t3 ^ t4;
        
    } while (p < end);
    
    global_checksum += sum;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum ^= x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (int)((uintptr_t)p & 0xFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += c;
}

void __attribute__((noinline)) use_long(long l) {
    global_checksum ^= (int)(l & 0xFFFFFFFF);
}

int main(int argc, char** argv) {
    /* Initialize test data */
    const size_t INT_ARRAY_SIZE = 1024;
    const size_t STRING_SIZE = 512;
    const size_t STRUCT_COUNT = 256;
    const size_t MATRIX_ROWS = 64;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* string_src = malloc(STRING_SIZE);
    char* string_dst = malloc(STRING_SIZE);
    struct Data* struct_array = malloc(STRUCT_COUNT * sizeof(struct Data));
    int (*matrix)[16] = malloc(MATRIX_ROWS * 16 * sizeof(int));
    char* mixed_data = malloc(STRING_SIZE * 2);
    
    if (!int_array || !string_src || !string_dst || !struct_array || !matrix || !mixed_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic data */
    for (size_t i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    for (size_t i = 0; i < STRING_SIZE; i++) {
        string_src[i] = 'A' + (i % 26);
    }
    
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = i * 3;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].timestamp = i * 1000;
        struct_array[i].weight = (i % 100) / 100.0f;
    }
    
    for (size_t i = 0; i < MATRIX_ROWS; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) * 7;
        }
    }
    
    for (size_t i = 0; i < STRING_SIZE * 2; i++) {
        mixed_data[i] = (i * 97) & 0xFF;
    }
    
    /* Run tests based on command line argument to prevent constant folding */
    int test_to_run = (argc > 1) ? atoi(argv[1]) % 6 : 0;
    
    int result1 = 0;
    long result3 = 0;
    int result4 = 0;
    
    switch (test_to_run) {
        case 0:
            result1 = test1_sum_int_array(int_array, INT_ARRAY_SIZE);
            printf("Test1 result: %d\n", result1);
            break;
        case 1:
            test2_copy_string(string_dst, string_src, STRING_SIZE);
            printf("Test2 copied string, first char: %c\n", string_dst[0]);
            break;
        case 2:
            result3 = test3_process_structs(struct_array, STRUCT_COUNT);
            printf("Test3 result: %ld\n", result3);
            break;
        case 3:
            result4 = test4_matrix_sum(matrix, MATRIX_ROWS);
            printf("Test4 result: %d\n", result4);
            break;
        case 4:
            test5_mixed_pointers(mixed_data, STRING_SIZE * 2, 3);
            printf("Test5 completed\n");
            break;
        default:
            /* Run all tests */
            result1 = test1_sum_int_array(int_array, INT_ARRAY_SIZE);
            test2_copy_string(string_dst, string_src, STRING_SIZE);
            result3 = test3_process_structs(struct_array, STRUCT_COUNT);
            result4 = test4_matrix_sum(matrix, MATRIX_ROWS);
            test5_mixed_pointers(mixed_data, STRING_SIZE * 2, 3);
            printf("All tests completed, checksum: %d\n", global_checksum);
            break;
    }
    
    /* Cleanup */
    free(int_array);
    free(string_src);
    free(string_dst);
    free(struct_array);
    free(matrix);
    free(mixed_data);
    
    return 0;
}
