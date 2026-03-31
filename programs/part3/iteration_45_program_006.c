/* test-auto-inc-dec.c
 * Program designed to trigger auto-increment/decrement optimization
 * targeting uncovered lines 1352-1358 in auto-inc-dec.cc
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
    int payload[3];
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_sum_int_array(const int* arr, size_t n) {
    int sum = 0;
    const int* p = arr;
    const int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    while (p < end) {
        /* Post-increment access - target pattern for find_inc() */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        temp1 += sum & 0xFF;
        temp2 += sum >> 8;
        temp3 = temp1 * temp2;
        temp4 = temp3 ^ temp1;
        temp5 = temp4 + temp2;
        temp6 = temp5 - temp3;
        temp7 = temp6 * 0x55;
        temp8 = temp7 ^ temp4;
        
        /* Prevent optimization of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all temps to prevent elimination */
    sum += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
    
    /* Opaque use to prevent optimization */
    use_int(sum);
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_copy_string(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    size_t i = 0;
    
    /* Register pressure variables */
    char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    int checksum = 0;
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        i++;
        
        /* Additional operations on the copied character */
        c1 = *(d - 1) ^ 0x55;
        c2 = c1 + 0x20;
        c3 = c2 * 2;
        c4 = c3 ^ c1;
        checksum += c1 + c2 + c3 + c4;
        
        /* Prevent optimization of pointers */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
    } while (i < n && *(s - 1) != '\0');
    
    /* Use results */
    use_char(c4);
    use_int(checksum);
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array(const struct Data* arr, size_t n) {
    int total = 0;
    const struct Data* p = arr;
    
    /* Many live variables for register pressure */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    int v5 = 0, v6 = 0, v7 = 0, v8 = 0;
    
    for (size_t i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->value;
        
        /* Complex computation with struct fields */
        v1 += p->tag;
        v2 += p->payload[0];
        v3 += p->payload[1];
        v4 += p->payload[2];
        
        /* Post-increment pointer after all accesses */
        const struct Data* old_p = p++;
        
        /* More computations to keep variables live */
        v5 = v1 * v2;
        v6 = v3 ^ v4;
        v7 = v5 + v6;
        v8 = v7 - v1;
        
        /* Prevent optimization */
        use_ptr((void*)old_p);
        asm volatile("" : : "r"(p) : "memory");
    }
    
    total += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int matrix[][10], size_t rows) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i = 0, j = 0;
    int* row_ptr = NULL;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        j = 0;
        
        /* Inner loop with post-increment on pointer */
        while (j < 10) {
            /* Access with pointer post-increment */
            sum += *row_ptr++;
            
            /* Additional computations */
            int t1 = sum & 0xF;
            int t2 = sum >> 4;
            int t3 = t1 * t2;
            int t4 = t3 ^ j;
            
            /* Use index in computation */
            sum += t4 * i;
            
            j++;
            
            /* Create register pressure */
            asm volatile("" : : "r"(row_ptr), "r"(j), "r"(i) : "memory");
        }
        
        /* Barrier to prevent loop optimizations */
        barrier();
    }
    
    return sum;
}

/* Test 5: Mixed pointer arithmetic with stride */
int test5_stride_access(const int* arr, size_t n, int stride) {
    int result = 0;
    const int* p = arr;
    size_t count = 0;
    
    /* Multiple stride calculations */
    int s1 = stride;
    int s2 = stride * 2;
    int s3 = stride * 3;
    int s4 = stride / 2;
    
    while (count < n) {
        /* Combined pointer arithmetic that may decompose to base+0 */
        result += *(p += stride);
        count++;
        
        /* Alternative form with explicit post-increment */
        if (count < n) {
            result += *p++;
            count++;
        }
        
        /* Use stride variables */
        result += s1 + s2 + s3 + s4;
        
        /* Modify strides to prevent constant propagation */
        s1 += 1;
        s2 -= 1;
        s3 ^= 0x1234;
        s4 *= 2;
        
        /* Heavy register pressure */
        asm volatile("" : : "r"(p), "r"(s1), "r"(s2), "r"(s3), "r"(s4) : "memory");
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize test data */
    const size_t INT_ARRAY_SIZE = 100;
    const size_t STRING_SIZE = 50;
    const size_t STRUCT_COUNT = 40;
    const size_t MATRIX_ROWS = 5;
    
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* string_src = malloc(STRING_SIZE);
    char* string_dst = malloc(STRING_SIZE);
    struct Data* struct_array = malloc(STRUCT_COUNT * sizeof(struct Data));
    int (*matrix)[10] = malloc(MATRIX_ROWS * 10 * sizeof(int));
    
    /* Initialize with non-constant data */
    for (size_t i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (int)(i * 3 + (argc > 1 ? atoi(argv[1]) : 1));
    }
    
    for (size_t i = 0; i < STRING_SIZE; i++) {
        string_src[i] = 'A' + (i % 26);
    }
    string_src[STRING_SIZE - 1] = '\0';
    
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = (int)i * 7;
        struct_array[i].tag = 'A' + (i % 26);
        for (int j = 0; j < 3; j++) {
            struct_array[i].payload[j] = (int)(i * 11 + j);
        }
    }
    
    for (size_t i = 0; i < MATRIX_ROWS; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (int)(i * 10 + j);
        }
    }
    
    /* Run tests based on command line to prevent constant folding */
    int test_to_run = (argc > 1) ? (atoi(argv[1]) % 5) : 0;
    int result = 0;
    
    switch (test_to_run) {
        case 0:
            result = test1_sum_int_array(int_array, INT_ARRAY_SIZE);
            break;
        case 1:
            test2_copy_string(string_dst, string_src, STRING_SIZE);
            result = string_dst[0] + string_dst[STRING_SIZE/2];
            break;
        case 2:
            result = test3_struct_array(struct_array, STRUCT_COUNT);
            break;
        case 3:
            result = test4_nested_loops(matrix, MATRIX_ROWS);
            break;
        case 4:
            result = test5_stride_access(int_array, INT_ARRAY_SIZE / 4, 2);
            break;
    }
    
    /* Update global volatile to ensure side effects */
    global_checksum += result;
    
    /* Cleanup */
    free(int_array);
    free(string_src);
    free(string_dst);
    free(struct_array);
    free(matrix);
    
    printf("Result: %d, Global checksum: %d\n", result, global_checksum);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum ^= x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (int)((size_t)p & 0xFFFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += c;
}

void __attribute__((noinline)) barrier(void) {
    asm volatile("" : : : "memory");
}
