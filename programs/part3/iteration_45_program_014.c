/* auto-inc-dec-test.c
 * Designed to trigger find_inc(true) path in GCC's auto-inc-dec optimization
 * Specifically targets lines 1352-1358 of auto-inc-dec.cc
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void barrier(void) __attribute__((noinline));

/* Prevent dead code elimination */
volatile int global_sum = 0;

/* Struct to create complex addressing */
struct Data {
    int value;
    char tag;
    short count;
    long timestamp;
};

/* Test 1: Integer array with post-increment pointer in loop */
int test1_post_increment_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    while (p < end) {
        /* Post-increment access - should create (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 += sum; r2 += r1; r3 += r2; r4 += r3; r5 += r4;
        r6 += r5; r7 += r6; r8 += r7; r9 += r8; r10 += r9;
        
        /* Prevent optimization of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all pressure variables */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Opaque use to prevent elimination */
    use_ptr(p);
    return sum;
}

/* Test 2: String copy with dual post-increment */
void test2_string_copy(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    
    /* Register pressure */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    
    for (size_t i = 0; i < n; i++) {
        /* Classic *dst++ = *src++ pattern */
        *d++ = *s++;
        
        /* Use pressure variables */
        c1++; c2++; c3++; c4++;
        i1 += c1; i2 += c2; i3 += c3; i4 += c4;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
    }
    
    /* Opaque uses */
    use_char(c1);
    use_int(i1 + i2 + i3 + i4);
}

/* Test 3: Struct array traversal with post-increment */
long test3_struct_array(struct Data* data, int count) {
    long total = 0;
    struct Data* p = data;
    
    /* Heavy register pressure */
    long l1 = 1, l2 = 2, l3 = 3, l4 = 4;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40;
    short s1 = 100, s2 = 200, s3 = 300, s4 = 400;
    
    for (int i = 0; i < count; i++) {
        /* Access struct member with post-increment */
        total += p->value;
        p++;  /* Post-increment after access */
        
        /* Complex pressure calculations */
        l1 += p->timestamp;
        l2 += l1;
        i1 += p->count;
        s1 += p->tag;
        
        /* Mix operations */
        l3 = (l3 * 1103515245 + 12345) & 0x7fffffff;
        i2 = (i2 * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total) : "memory");
    }
    
    total += l1 + l2 + l3 + l4 + i1 + i2 + i3 + i4 + s1 + s2 + s3 + s4;
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for pressure */
    int i, j, k = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;
    int* row_ptr;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        
        /* Inner loop with pointer arithmetic */
        for (j = 0; j < cols; j++) {
            /* Combined form that may decompose to base+offset */
            sum += *(row_ptr + j);
            
            /* Alternative: post-increment in separate statement */
            tmp1 += *row_ptr;
            row_ptr++;  /* Post-increment */
            
            /* More pressure */
            tmp2 += tmp1;
            tmp3 += tmp2;
            tmp4 += tmp3;
            k = (k + 1) & 31;
        }
        
        /* Opaque function call to create aliasing */
        use_ptr(matrix[i]);
    }
    
    sum += tmp1 + tmp2 + tmp3 + tmp4 + k;
    return sum;
}

/* Test 5: Mixed pointer types with stride */
int test5_mixed_pointers(char* base, int stride, int iterations) {
    int sum = 0;
    char* p = base;
    int* ip;
    
    /* Different pointer types */
    short* sp = (short*)base;
    long* lp = (long*)base;
    
    for (int i = 0; i < iterations; i++) {
        /* Access through different pointer types */
        sum += *p;          /* char access */
        p += stride;        /* Pointer increment with stride */
        
        sum += *sp;         /* short access */
        sp += stride;       /* May create different offset patterns */
        
        ip = (int*)p;       /* Cast to int pointer */
        sum += *ip;
        
        lp = (long*)sp;     /* Cast to long pointer */
        sum += *lp;
        
        /* Complex address calculation */
        asm volatile("" : : "r"(p), "r"(sp), "r"(ip), "r"(lp) : "memory");
    }
    
    return sum;
}

/* Test 6: do-while loop with post-decrement */
int test6_do_while_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;  /* Start from end */
    
    /* Register pressure variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    
    do {
        /* Post-decrement access */
        sum += *p--;
        
        /* Pressure calculations */
        a = a * 3 + sum;
        b = b * 5 + a;
        c = c * 7 + b;
        d = d * 11 + c;
        e = e * 13 + d;
        
    } while (p >= arr);
    
    sum += a + b + c + d + e;
    use_ptr(p);
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 32;
    const int MATRIX_COLS = 32;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = (struct Data*)malloc(ARRAY_SIZE * sizeof(struct Data));
    int** matrix = (int**)malloc(MATRIX_ROWS * sizeof(int*));
    
    if (!int_array || !char_array || !struct_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        char_array[i] = (char)(i % 256);
        struct_array[i].value = i;
        struct_array[i].tag = (char)(i % 128);
        struct_array[i].count = (short)(i % 1000);
        struct_array[i].timestamp = i * 1000L;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = (int*)malloc(MATRIX_COLS * sizeof(int));
        if (matrix[i]) {
            for (int j = 0; j < MATRIX_COLS; j++) {
                matrix[i][j] = i * MATRIX_COLS + j;
            }
        }
    }
    
    /* Select test based on command line argument */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    switch (test_num) {
        case 1:
            result = test1_post_increment_sum(int_array, ARRAY_SIZE);
            break;
        case 2:
            test2_string_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result = char_array[0];
            break;
        case 3:
            result = (int)test3_struct_array(struct_array, ARRAY_SIZE);
            break;
        case 4:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 5:
            result = test5_mixed_pointers(char_array, 3, ARRAY_SIZE/4);
            break;
        case 6:
            result = test6_do_while_decrement(int_array, ARRAY_SIZE);
            break;
        default:
            /* Run all tests */
            result = test1_post_increment_sum(int_array, ARRAY_SIZE);
            test2_string_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result += (int)test3_struct_array(struct_array, ARRAY_SIZE);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test5_mixed_pointers(char_array, 3, ARRAY_SIZE/4);
            result += test6_do_while_decrement(int_array, ARRAY_SIZE);
            break;
    }
    
    /* Update global to prevent elimination */
    global_sum = result;
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(char_array);
    free(struct_array);
    
    printf("Result: %d\n", result);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_sum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    if (p) global_sum += 1;
}

void __attribute__((noinline)) use_char(char c) {
    global_sum += c;
}

void __attribute__((noinline)) barrier(void) {
    asm volatile("" : : : "memory");
}
