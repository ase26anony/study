/* test_auto_inc_dec.c - Target program for auto-inc-dec.cc coverage */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void use_struct(void*) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct for testing pointer arithmetic */
struct TestStruct {
    int id;
    char data[8];
    float value;
};

/* Test 1: Integer array summation with post-increment pointer */
int test_int_array_sum(int* arr, size_t n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* Pattern: *p++ creates mem_insn with (plus (reg) (const_int 0)) */
        sum += *p++;  /* This should generate the target RTL pattern */
        
        /* Use temp variables to create register pressure */
        temp1 += sum & 0xFF;
        temp2 += sum >> 8;
        temp3 = temp1 * temp2;
        temp4 = temp3 ^ sum;
        temp5 = temp4 + *arr;  /* Use arr to keep it live */
        temp6 = temp5 - temp3;
        temp7 = temp6 | temp4;
        temp8 = temp7 & 0x7F;
    }
    
    /* Make all variables appear used */
    asm volatile("" : : "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4),
                     "r"(temp5), "r"(temp6), "r"(temp7), "r"(temp8));
    
    use_ptr(p);  /* Prevent elimination of pointer */
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test_string_copy(char* dst, const char* src, size_t len) {
    char* d = dst;
    const char* s = src;
    const char* end = src + len;
    
    /* Register pressure variables */
    char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    int count = 0;
    
    /* Classic *dst++ = *src++ pattern */
    while (s < end) {
        /* Both sides use post-increment */
        *d++ = *s++;  /* Should generate target pattern for both load and store */
        
        /* Additional operations to prevent over-optimization */
        c1 = *src;  /* Keep src live with different access */
        c2 = c1 ^ count;
        c3 = c2 + *dst;
        c4 = c3 - c1;
        count++;
        
        /* Use asm to prevent elimination */
        asm volatile("" : : "r"(c1), "r"(c2), "r"(c3), "r"(c4));
    }
    
    use_ptr(d);
    use_ptr(s);
}

/* Test 3: Struct array traversal with post-increment */
float test_struct_array(struct TestStruct* arr, size_t n) {
    struct TestStruct* p = arr;
    struct TestStruct* end = arr + n;
    float total = 0.0f;
    
    /* Many live variables for register pressure */
    int idsum = 0;
    char databuf[8] = {0};
    float f1 = 0.0f, f2 = 0.0f, f3 = 0.0f, f4 = 0.0f;
    
    /* Loop accessing struct members via post-increment pointer */
    for (; p < end; p++) {
        /* Access struct member - pointer arithmetic happens in p++ */
        total += p->value;  /* Base access */
        idsum += p->id;
        
        /* Copy data to create more memory ops */
        for (int i = 0; i < 8; i++) {
            databuf[i] ^= p->data[i];
        }
        
        /* More register pressure */
        f1 = total * 1.1f;
        f2 = f1 + p->value;
        f3 = f2 - total;
        f4 = f3 * 0.9f;
        
        asm volatile("" : : "r"(f1), "r"(f2), "r"(f3), "r"(f4));
    }
    
    /* Force use of variables */
    use_int(idsum);
    use_struct(databuf);
    return total + idsum + f4;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Outer loop with pointer */
    for (int i = 0; i < rows; i++) {
        int* row = matrix[i];
        
        /* Inner loop with post-increment on index calculation */
        for (int j = 0; j < cols; j++) {
            /* Access with offset that could be optimized */
            sum += row[j];  /* May become *(row + j) then j++ in loop update */
            
            /* Additional index arithmetic */
            int idx = j * 2;
            if (idx < cols) {
                sum -= row[idx];  /* Another memory access pattern */
            }
        }
        
        /* Post-increment style operation in loop update */
        matrix[i] = row;  /* Keep pointer alive */
    }
    
    /* Create complex addressing pattern */
    int* base = matrix[0];
    for (int offset = 0; offset < cols; offset += 2) {
        /* Pattern: *(base + offset) with offset changing */
        sum ^= *(base + offset);  /* plus (reg) (const_int offset) */
        
        /* Make offset calculation non-trivial */
        asm volatile("" : : "r"(offset));
    }
    
    return sum;
}

/* Test 5: Mixed pointer types and arithmetic */
void test_mixed_pointers(void* data, size_t size) {
    char* cptr = (char*)data;
    int* iptr = (int*)data;
    struct TestStruct* sptr = (struct TestStruct*)data;
    
    size_t int_count = size / sizeof(int);
    size_t struct_count = size / sizeof(struct TestStruct);
    
    /* Process as integers with post-increment */
    for (size_t i = 0; i < int_count; i++) {
        int val = *iptr++;
        global_checksum += val;
        
        /* Also access as chars to create aliasing */
        char* temp = cptr + i * sizeof(int);
        for (int j = 0; j < sizeof(int); j++) {
            global_checksum += temp[j];
        }
    }
    
    /* Process as structs */
    for (size_t i = 0; i < struct_count; i++) {
        global_checksum += sptr->id;
        sptr++;  /* Post-increment struct pointer */
    }
    
    /* Complex pointer arithmetic */
    char* end = cptr + size;
    while (cptr < end) {
        /* Various access patterns */
        *cptr = (*cptr) ^ 0x55;
        cptr += 3;  /* Non-unit stride */
        
        /* Check alignment for int access */
        if (((uintptr_t)cptr & 0x3) == 0 && cptr + sizeof(int) <= end) {
            int* aligned = (int*)cptr;
            *aligned = *aligned + 1;
        }
    }
}

/* Main driver with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const size_t INT_ARRAY_SIZE = 1024;
    const size_t CHAR_ARRAY_SIZE = 2048;
    const size_t STRUCT_COUNT = 128;
    const size_t MATRIX_ROWS = 16;
    const size_t MATRIX_COLS = 32;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(INT_ARRAY_SIZE * sizeof(int));
    char* char_array = malloc(CHAR_ARRAY_SIZE);
    struct TestStruct* struct_array = malloc(STRUCT_COUNT * sizeof(struct TestStruct));
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    
    if (!int_array || !char_array || !struct_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant data */
    for (size_t i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 37) & 0xFF;  /* Non-trivial pattern */
    }
    
    for (size_t i = 0; i < CHAR_ARRAY_SIZE; i++) {
        char_array[i] = (char)(i ^ (i >> 4));
    }
    
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].id = (int)i;
        for (int j = 0; j < 8; j++) {
            struct_array[i].data[j] = (char)(i + j);
        }
        struct_array[i].value = (float)i * 0.5f;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    /* Use command-line argument to select tests */
    int test_mask = 0x1F;  /* Run all tests by default */
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    }
    
    int total_result = 0;
    
    /* Run tests based on mask */
    if (test_mask & 0x01) {
        total_result += test_int_array_sum(int_array, INT_ARRAY_SIZE);
    }
    
    if (test_mask & 0x02) {
        char* dst = malloc(CHAR_ARRAY_SIZE);
        test_string_copy(dst, char_array, CHAR_ARRAY_SIZE);
        free(dst);
    }
    
    if (test_mask & 0x04) {
        total_result += (int)test_struct_array(struct_array, STRUCT_COUNT);
    }
    
    if (test_mask & 0x08) {
        total_result += test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
    }
    
    if (test_mask & 0x10) {
        test_mixed_pointers(int_array, INT_ARRAY_SIZE * sizeof(int));
    }
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    
    free(int_array);
    free(char_array);
    free(struct_array);
    free(matrix);
    
    /* Use result to prevent elimination */
    printf("Result: %d (checksum: %d)\n", total_result, global_checksum);
    
    return total_result != 0 ? 0 : 1;
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

void __attribute__((noinline)) use_struct(void* s) {
    global_checksum += ((char*)s)[0];
}
