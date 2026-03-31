/* test_auto_inc_dec.c - Target coverage for auto-inc-dec.cc lines 1352-1358 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern int get_value(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_sum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int id;
    float weight;
};

/* Prevent compiler from seeing implementation */
__attribute__((noinline))
void use_int(int x) {
    global_sum += x;
    asm volatile("" : : "r"(x));
}

__attribute__((noinline))
void use_ptr(void* p) {
    asm volatile("" : : "r"(p));
}

__attribute__((noinline))
void use_char(char c) {
    global_sum += c;
    asm volatile("" : : "r"(c));
}

__attribute__((noinline))
int get_value(void) {
    return rand() % 100;
}

/* Test 1: Integer array summation with post-increment pointer in loop */
__attribute__((noinline))
int test_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = get_value();
    int temp2 = get_value();
    int temp3 = get_value();
    int temp4 = get_value();
    int temp5 = get_value();
    int temp6 = get_value();
    int temp7 = get_value();
    int temp8 = get_value();
    
    while (p < end) {
        /* Post-increment access - target pattern for find_inc() */
        sum += *p++;
        
        /* Use all temporaries to maintain register pressure */
        temp1 += sum;
        temp2 += temp1;
        temp3 += temp2;
        temp4 += temp3;
        temp5 += temp4;
        temp6 += temp5;
        temp7 += temp6;
        temp8 += temp7;
        
        /* Opaque use to prevent optimization */
        use_ptr(p);
    }
    
    /* Consume temporaries */
    sum += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
    return sum;
}

/* Test 2: String copy with post-increment on both source and dest */
__attribute__((noinline))
void test_char_copy(char* dest, const char* src, int n) {
    char* d = dest;
    const char* s = src;
    int i = 0;
    
    /* Register pressure variables */
    char c1, c2, c3, c4;
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Additional operations to create complex RTL */
        c1 = *(s - 1);
        c2 = *(d - 1);
        t1 += c1;
        t2 += c2;
        t3 += i;
        t4 += t1 + t2 + t3;
        
        /* Opaque use to prevent elimination */
        use_char(c1);
        use_char(c2);
        
        i++;
    } while (i < n);
    
    global_sum += t1 + t2 + t3 + t4;
}

/* Test 3: Struct array traversal with post-increment */
__attribute__((noinline))
int test_struct_array(struct Data* arr, int n) {
    int sum = 0;
    struct Data* p = arr;
    int count = 0;
    
    /* High register pressure */
    int r1 = get_value(), r2 = get_value(), r3 = get_value();
    int r4 = get_value(), r5 = get_value(), r6 = get_value();
    float f1 = 0.0f, f2 = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member via post-incrementing pointer */
        sum += p->value;
        f1 += p->weight;
        
        /* Post-increment the pointer */
        struct Data* old_p = p++;
        
        /* Complex operations to maintain register pressure */
        r1 += old_p->id;
        r2 += p->tag;
        r3 += sum;
        r4 += r1 + r2;
        r5 += (int)f1;
        r6 += i * 2;
        
        /* Force pointer to appear used */
        use_ptr(old_p);
        use_ptr(p);
    }
    
    sum += r1 + r2 + r3 + r4 + r5 + r6 + (int)f1;
    return sum;
}

/* Test 4: Nested loops with array indexing and post-increment */
__attribute__((noinline))
int test_nested_loops(int* matrix, int rows, int cols) {
    int total = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l, m, n;
    k = l = m = n = 0;
    
    for (i = 0; i < rows; i++) {
        int* row_ptr = matrix + i * cols;
        int* row_end = row_ptr + cols;
        
        /* Inner loop with pointer arithmetic */
        for (j = 0; j < cols; j++) {
            /* Combined form that may decompose to base+offset */
            total += *(row_ptr + j);
            
            /* Additional post-increment in update expression */
            k += *(row_ptr++);
            
            /* More register pressure */
            l += i;
            m += j;
            n += total;
            
            /* Prevent optimization */
            if (j % 3 == 0) {
                use_ptr(row_ptr);
            }
        }
        
        /* Complex update to prevent simple loop optimization */
        total += k - l + m - n;
        k = l = m = n = 0;
    }
    
    return total;
}

/* Test 5: Mixed pointer types with stride access */
__attribute__((noinline))
int test_mixed_pointers(void* base, int elements, int stride) {
    char* byte_ptr = (char*)base;
    int sum = 0;
    
    /* Multiple typed pointers for different access patterns */
    int* int_ptr = (int*)base;
    struct Data* struct_ptr = (struct Data*)base;
    
    /* Register pressure variables */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    
    for (int i = 0; i < elements; i++) {
        /* Access through byte pointer with stride */
        sum += *(int*)(byte_ptr);
        
        /* Post-increment with stride - may create (plus (reg) (const_int 0)) */
        byte_ptr += stride;
        
        /* Alternate access through int pointer */
        a += *int_ptr++;
        
        /* Every 4th iteration, use struct pointer */
        if (i % 4 == 0) {
            b += struct_ptr->value;
            struct_ptr++;
        }
        
        /* Maintain register pressure */
        c += sum;
        d += a;
        e += b;
        f += i;
        
        /* Opaque uses */
        use_ptr(byte_ptr);
        use_ptr(int_ptr);
    }
    
    return sum + a + b + c + d + e + f;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize data */
    const int INT_ARRAY_SIZE = 1024;
    const int CHAR_ARRAY_SIZE = 512;
    const int STRUCT_COUNT = 256;
    const int MATRIX_ROWS = 32;
    const int MATRIX_COLS = 32;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(INT_ARRAY_SIZE * sizeof(int));
    char* char_array1 = (char*)malloc(CHAR_ARRAY_SIZE);
    char* char_array2 = (char*)malloc(CHAR_ARRAY_SIZE);
    struct Data* struct_array = (struct Data*)malloc(STRUCT_COUNT * sizeof(struct Data));
    int* matrix = (int*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * 37) % 101;
    }
    
    for (int i = 0; i < CHAR_ARRAY_SIZE; i++) {
        char_array1[i] = 'A' + (i % 26);
        char_array2[i] = 0;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = i * 2;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].id = i;
        struct_array[i].weight = i * 0.5f;
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = (i * 19) % 97;
    }
    
    int result = 0;
    
    /* Use command-line argument to select test, preventing constant folding */
    int test_selector = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_selector % 5) {
        case 0:
            result = test_int_array_sum(int_array, INT_ARRAY_SIZE);
            break;
        case 1:
            test_char_copy(char_array2, char_array1, CHAR_ARRAY_SIZE);
            result = char_array2[0] + char_array2[CHAR_ARRAY_SIZE-1];
            break;
        case 2:
            result = test_struct_array(struct_array, STRUCT_COUNT);
            break;
        case 3:
            result = test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            result = test_mixed_pointers(int_array, INT_ARRAY_SIZE / 4, sizeof(int) * 2);
            break;
    }
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", result);
    printf("Global sum: %d\n", global_sum);
    
    /* Cleanup */
    free(int_array);
    free(char_array1);
    free(char_array2);
    free(struct_array);
    free(matrix);
    
    return 0;
}
