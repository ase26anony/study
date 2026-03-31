/* test-auto-inc-dec.c
 * Designed to trigger find_inc(true) path in GCC's auto-inc-dec pass
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
volatile int global_checksum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int payload[2];
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_sum_int_array(const int* arr, size_t n) {
    register int sum = 0;  /* register keyword increases pressure */
    const int* p = arr;
    int i = 0;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    /* Loop with post-increment - should generate (plus (reg) (const_int 0)) */
    while (i++ < n) {
        /* The key pattern: *p++ where p is modified after fetch */
        sum += *p++;
        
        /* Use all temp variables to maintain register pressure */
        temp1 += sum & 0xFF;
        temp2 += sum >> 8;
        temp3 += i * 2;
        temp4 += i * 3;
        temp5 ^= sum;
        temp6 |= temp1;
        temp7 &= temp2;
        temp8 = temp3 + temp4;
        
        /* Prevent optimization of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all variables to prevent elimination */
    int result = sum + temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
    use_int(result);
    return result;
}

/* Test 2: String copy with post-increment on both pointers */
void test2_str_copy(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    size_t i = 0;
    
    /* Register pressure variables */
    register int checksum = 0;
    int live1 = 0, live2 = 0, live3 = 0, live4 = 0;
    
    /* Classic *dst++ = *src++ pattern */
    do {
        /* Post-increment on both load and store */
        *d++ = *s++;
        
        /* Maintain register pressure */
        checksum += *s;
        live1 += checksum;
        live2 += i;
        live3 = live1 ^ live2;
        live4 = live3 + checksum;
        
        /* Make pointers appear used */
        use_ptr(d);
        use_ptr(s);
    } while (++i < n);
    
    /* Use results */
    global_checksum += checksum + live1 + live2 + live3 + live4;
    barrier();
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array(const struct Data* arr, size_t n) {
    const struct Data* p = arr;
    int total = 0;
    
    /* High register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    
    for (size_t i = 0; i < n; ++i) {
        /* Access struct member via post-incrementing pointer */
        total += p->value;
        
        /* Complex expression to prevent optimization */
        r1 += p->tag;
        r2 += p->payload[0];
        r3 += p->payload[1];
        r4 = r1 * r2;
        r5 = r3 ^ r4;
        r6 += total;
        r7 = r5 + r6;
        r8 = r7 - r1;
        r9 = r8 * 2;
        r10 = r9 / 3;
        
        /* Post-increment the struct pointer */
        p++;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(p), "r"(total) : "memory");
    }
    
    /* Combine all results */
    int result = total + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    use_int(result);
    return result;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int* matrix, int rows, int cols) {
    int sum = 0;
    
    /* Many live variables for register pressure */
    register int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;
    
    for (int i = 0; i < rows; ++i) {
        int* row_ptr = matrix + i * cols;
        int j = 0;
        
        /* Inner loop with pointer arithmetic */
        while (j < cols) {
            /* Combined form that may decompose to base+0 */
            sum += *(row_ptr + j);
            
            /* Post-increment index in separate expression */
            j++;
            
            /* Maintain many live variables */
            acc1 += sum;
            acc2 += j;
            acc3 = acc1 ^ acc2;
            tmp1 = acc3 * 2;
            tmp2 = tmp1 + sum;
            tmp3 = tmp2 - j;
            tmp4 = tmp3 & 0xFF;
            
            /* Opaque use of variables */
            use_int(tmp4);
        }
        
        /* Complex pointer update that may create opportunities */
        matrix = matrix + cols;
        use_ptr(matrix);
    }
    
    return sum + acc1 + acc2 + acc3 + tmp1 + tmp2 + tmp3 + tmp4;
}

/* Test 5: Mixed pointer types with stride access */
int test5_mixed_pointers(char* data, int size, int stride) {
    char* p = data;
    int sum = 0;
    
    /* Extreme register pressure */
    int v[16];
    for (int i = 0; i < 16; i++) v[i] = i;
    
    for (int i = 0; i < size; i += stride) {
        /* Pointer with stride - may create (plus (reg) (const_int 0)) */
        char* current = p + i;
        
        /* Access with potential post-increment pattern */
        sum += *current;
        
        /* Update pointer in way that might be optimized */
        p = current;
        
        /* Use all pressure variables */
        for (int j = 0; j < 16; j++) {
            v[j] += sum + j;
            asm volatile("" : : "r"(v[j]) : "memory");
        }
        
        /* Force pointer to stay live */
        use_ptr(p);
    }
    
    /* Combine all variables */
    int total = sum;
    for (int i = 0; i < 16; i++) total += v[i];
    return total;
}

/* Test 6: do-while loop with post-decrement */
int test6_post_decrement(int* arr, size_t n) {
    int* p = arr + n - 1;
    int sum = 0;
    size_t count = n;
    
    /* Register pressure */
    int a = 0, b = 0, c = 0, d = 0, e = 0;
    
    do {
        /* Post-decrement access */
        sum += *p--;
        
        /* Complex computations to maintain variables */
        a += sum & 1;
        b += count;
        c = a ^ b;
        d = c * sum;
        e = d >> 2;
        
        /* Memory barrier */
        asm volatile("" : : "r"(p), "r"(sum) : "memory");
    } while (count-- > 0);
    
    return sum + a + b + c + d + e;
}

/* Dummy implementations of opaque functions */
void use_int(int x) {
    global_checksum ^= x;
}

void use_ptr(void* p) {
    global_checksum += (long)p & 0xFF;
}

void use_char(char c) {
    global_checksum += c;
}

void barrier(void) {
    asm volatile("" : : : "memory");
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 32;
    const int MATRIX_COLS = 32;
    
    /* Allocate and initialize arrays */
    int* int_array = malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = malloc(ARRAY_SIZE * sizeof(struct Data));
    int* matrix = malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    if (!int_array || !char_array || !struct_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        char_array[i] = (i % 26) + 'a';
        struct_array[i].value = i * 2;
        struct_array[i].tag = i % 128;
        struct_array[i].payload[0] = i;
        struct_array[i].payload[1] = i * i;
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = i % 100;
    }
    
    /* Select test based on command line argument */
    int test_num = (argc > 1) ? atoi(argv[1]) % 7 : 0;
    int result = 0;
    
    switch (test_num) {
        case 0:
            result = test1_sum_int_array(int_array, ARRAY_SIZE);
            break;
        case 1:
            test2_str_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result = global_checksum;
            break;
        case 2:
            result = test3_struct_array(struct_array, ARRAY_SIZE);
            break;
        case 3:
            result = test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            result = test5_mixed_pointers(char_array, ARRAY_SIZE, 3);
            break;
        case 5:
            result = test6_post_decrement(int_array, ARRAY_SIZE);
            break;
        default:
            /* Run all tests */
            result = test1_sum_int_array(int_array, ARRAY_SIZE);
            test2_str_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result += test3_struct_array(struct_array, ARRAY_SIZE);
            result += test4_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            result += test5_mixed_pointers(char_array, ARRAY_SIZE, 3);
            result += test6_post_decrement(int_array, ARRAY_SIZE);
            break;
    }
    
    /* Clean up */
    free(int_array);
    free(char_array);
    free(struct_array);
    free(matrix);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
