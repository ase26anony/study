/* test-auto-inc-dec.c
 * Program designed to trigger auto-increment/decrement optimization patterns
 * Specifically targets find_inc() logic with mem_loc = address_of_x, reg0 = XEXP(x,0), reg1_is_const = true
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void barrier(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Opaque function definitions */
void use_int(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void use_ptr(void* p) {
    asm volatile("" : : "r"(p) : "memory");
}

void use_char(char c) {
    asm volatile("" : : "r"(c) : "memory");
}

void barrier(void) {
    asm volatile("" : : : "memory");
}

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    short count;
    int data[3];
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0;
    
    while (p < end) {
        /* Post-increment access - should generate (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 += sum & 1;
        r2 += sum & 2;
        r3 += sum & 4;
        r4 += sum & 8;
        r5 += sum & 16;
        r6 += sum & 32;
        r7 += sum & 64;
        r8 += sum & 128;
        
        /* Prevent optimization */
        use_ptr(p);
    }
    
    /* Combine all register pressure variables */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_string_copy(char* dst, const char* src, size_t len) {
    char* d = dst;
    const char* s = src;
    size_t i = 0;
    
    /* Register pressure variables */
    char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Additional operations to prevent over-optimization */
        c1 = *s;
        c2 = *(s - 1);
        c3 = *d;
        c4 = *(d - 1);
        
        t1 += c1;
        t2 += c2;
        t3 += c3;
        t4 += c4;
        
        /* Use asm to prevent elimination */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
        
        i++;
    } while (i < len);
    
    /* Use variables */
    use_char(c1 + c2 + c3 + c4);
    use_int(t1 + t2 + t3 + t4);
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array_sum(struct Data* arr, int n) {
    int sum = 0;
    struct Data* p = arr;
    
    /* Heavy register pressure */
    int accum[8] = {0};
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        sum += p->value;
        
        /* Complex addressing to create (plus (reg) (const_int 0)) pattern */
        int* data_ptr = p->data;
        tmp1 = data_ptr[0];
        tmp2 = data_ptr[1];
        tmp3 = data_ptr[2];
        
        /* Post-increment the struct pointer */
        p++;
        
        /* Use all temporaries */
        accum[0] += tmp1;
        accum[1] += tmp2;
        accum[2] += tmp3;
        accum[3] += p[-1].tag;
        accum[4] += p[-1].count;
        
        /* More register pressure */
        tmp4 = accum[0];
        tmp5 = accum[1];
        tmp6 = accum[2];
        tmp7 = accum[3];
        tmp8 = accum[4];
        
        /* Prevent optimization */
        use_ptr(p);
        use_int(tmp4 + tmp5 + tmp6 + tmp7 + tmp8);
    }
    
    /* Combine accumulators */
    for (int j = 0; j < 5; j++) {
        sum += accum[j];
    }
    
    return sum;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int matrix[][10], int rows, int cols) {
    int total = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l, m, n, o, p;
    i = j = k = l = m = n = o = p = 0;
    
    for (i = 0; i < rows; i++) {
        int* row_ptr = matrix[i];
        int* end_ptr = row_ptr + cols;
        
        /* Inner loop with pointer arithmetic */
        while (row_ptr < end_ptr) {
            /* Combined form that may decompose to base+offset */
            total += *(row_ptr += 1) - 1;  /* Actually increments then accesses */
            
            /* Adjust for correct pointer position */
            row_ptr--;
            
            /* Post-increment access - target pattern */
            total += *row_ptr++;
            
            /* More register pressure */
            j += total & 1;
            k += total & 2;
            l += total & 4;
            m += total & 8;
            n += total & 16;
            o += total & 32;
            p += total & 64;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(row_ptr), "r"(end_ptr) : "memory");
        }
    }
    
    total += j + k + l + m + n + o + p;
    return total;
}

/* Test 5: Mixed pointer types and arithmetic */
int test5_mixed_pointers(char* cptr, int* iptr, short* sptr, int n) {
    int sum = 0;
    
    /* Multiple pointers with different types */
    char* cp = cptr;
    int* ip = iptr;
    short* sp = sptr;
    
    /* Register pressure variables */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0, v6 = 0, v7 = 0, v8 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Post-increment accesses with different pointer types */
        v1 += (int)(*cp++);
        v2 += *ip++;
        v3 += (int)(*sp++);
        
        /* Additional pointer arithmetic */
        v4 += *(cp - 1);
        v5 += *(ip - 1);
        v6 += *(sp - 1);
        
        /* More complex patterns */
        v7 += *(cp += 1) - 1;
        cp--;
        v8 += *(ip += 1) - 1;
        ip--;
        
        /* Use all variables */
        use_ptr(cp);
        use_ptr(ip);
        use_ptr(sp);
    }
    
    sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 100;
    const int MATRIX_ROWS = 10;
    const int MATRIX_COLS = 10;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_array1 = (char*)malloc(ARRAY_SIZE * sizeof(char));
    char* char_array2 = (char*)malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = (struct Data*)malloc(ARRAY_SIZE * sizeof(struct Data));
    int (*matrix)[MATRIX_COLS] = (int(*)[MATRIX_COLS])malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 100;
        char_array1[i] = 'A' + (i % 26);
        char_array2[i] = 0;
        
        struct_array[i].value = i * 2;
        struct_array[i].tag = 'a' + (i % 26);
        struct_array[i].count = i % 100;
        for (int j = 0; j < 3; j++) {
            struct_array[i].data[j] = i * 10 + j;
        }
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = i * MATRIX_COLS + j;
        }
    }
    
    int result = 0;
    
    /* Use command-line arguments to select tests */
    int test_to_run = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_to_run) {
        case 1:
            result = test1_int_array_sum(int_array, ARRAY_SIZE);
            break;
        case 2:
            test2_string_copy(char_array2, char_array1, ARRAY_SIZE);
            result = char_array2[0] + char_array2[ARRAY_SIZE-1];
            break;
        case 3:
            result = test3_struct_array_sum(struct_array, ARRAY_SIZE);
            break;
        case 4:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 5:
            result = test5_mixed_pointers(char_array1, int_array, 
                                         (short*)int_array, ARRAY_SIZE/2);
            break;
        default:
            /* Run all tests */
            result += test1_int_array_sum(int_array, ARRAY_SIZE);
            test2_string_copy(char_array2, char_array1, ARRAY_SIZE);
            result += test3_struct_array_sum(struct_array, ARRAY_SIZE);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test5_mixed_pointers(char_array1, int_array, 
                                          (short*)int_array, ARRAY_SIZE/2);
            break;
    }
    
    /* Update global volatile to prevent elimination */
    global_checksum = result;
    
    /* Print result to ensure code isn't dead */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(int_array);
    free(char_array1);
    free(char_array2);
    free(struct_array);
    free(matrix);
    
    return 0;
}
