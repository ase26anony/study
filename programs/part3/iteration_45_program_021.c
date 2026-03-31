/* test-auto-inc-dec.c
 * Program designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358 in auto-inc-dec.cc: find_inc(true) path for mem_insn with reg+0 offset
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void barrier(void) __attribute__((noinline, noipa));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int payload[2];
};

/* Test 1: Integer array summation with post-increment pointer in loop */
int test1_sum_int_array(const int* arr, size_t n) {
    int sum = 0;
    const int* p = arr;
    const int* end = arr + n;
    
    /* Create register pressure with many live variables */
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* Pattern: *p++ creates mem access with (plus (reg) (const_int 0)) */
        sum += *p++;  /* This should generate the target RTL pattern */
        
        /* Use all live variables to increase register pressure */
        r0 += sum & 1;
        r1 += sum & 2;
        r2 += sum & 4;
        r3 += sum & 8;
        r4 += (sum >> 4) & 1;
        r5 += (sum >> 5) & 1;
        r6 += (sum >> 6) & 1;
        r7 += (sum >> 7) & 1;
        
        /* Prevent loop unrolling from eliminating the pattern */
        asm volatile("" : : "r"(p), "r"(sum) : "memory");
    }
    
    /* Combine all register pressure variables */
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    
    /* Use opaque function to prevent optimization */
    use_int(sum);
    
    return sum;
}

/* Test 2: String copy with *dst++ = *src++ pattern */
void test2_copy_string(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    size_t i = 0;
    
    /* Register pressure variables */
    char c0 = 0, c1 = 0, c2 = 0, c3 = 0;
    int checksum = 0;
    
    /* do-while loop ensures at least one iteration */
    if (n > 0) {
        do {
            /* Classic post-increment copy pattern */
            *d++ = *s++;  /* Should generate target RTL for both load and store */
            
            /* Use the copied value to prevent elimination */
            checksum += *(d - 1);
            
            /* More register pressure */
            c0 = (c0 + 1) & 0x7F;
            c1 = (c1 + 2) & 0x7F;
            c2 = (c2 + 3) & 0x7F;
            c3 = (c3 + 4) & 0x7F;
            
            /* Memory barrier to prevent reordering */
            asm volatile("" : : "r"(d), "r"(s) : "memory");
            
            i++;
        } while (i < n);
    }
    
    /* Use all variables */
    checksum += c0 + c1 + c2 + c3;
    use_int(checksum);
    use_ptr(dst);
    use_ptr(src);
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array(const struct Data* arr, size_t n) {
    int total = 0;
    const struct Data* p = arr;
    
    /* High register pressure */
    int accum[8] = {0};
    int temp1, temp2, temp3, temp4;
    
    for (size_t i = 0; i < n; i++) {
        /* Access struct member via post-increment pointer */
        total += p->value;  /* Base access */
        
        /* Complex addressing that might decompose to reg+0 */
        temp1 = p->payload[0];
        temp2 = p->payload[1];
        
        /* Post-increment the pointer */
        p++;  /* Separate increment instruction */
        
        /* Use all temporaries to keep them live */
        accum[0] += temp1;
        accum[1] += temp2;
        accum[2] += p[-1].tag;  /* Use previous element */
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total) : "memory");
        
        /* More operations to create register pressure */
        for (int j = 0; j < 4; j++) {
            accum[j + 3] += (total >> j) & 1;
        }
    }
    
    /* Combine accumulators */
    for (int j = 0; j < 8; j++) {
        total += accum[j];
    }
    
    use_int(total);
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int matrix[][10], int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l, m, n;
    int* row_ptr;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        
        /* Inner loop with pointer arithmetic */
        for (j = 0; j < cols; j++) {
            /* Access with offset that might become reg+0 */
            sum += row_ptr[j];  /* Could be *(row_ptr + j*4) */
            
            /* Additional index calculations */
            k = (i * cols + j) & 7;
            l = (sum >> k) & 1;
            m = (j * 17) & 31;
            n = (i * 23) & 31;
            
            /* Use all variables */
            sum += k + l + m + n;
            
            /* Memory barrier */
            asm volatile("" : : "r"(row_ptr), "r"(j) : "memory");
        }
        
        /* Post-increment style operation in loop update */
        /* This creates separate arithmetic instructions */
        row_ptr += cols;  /* Not used, but creates more RTL patterns */
    }
    
    /* Alternative: while loop with explicit post-increment */
    int* flat = &matrix[0][0];
    int* end_flat = flat + rows * cols;
    int checksum = 0;
    
    while (flat < end_flat) {
        checksum ^= *flat++;  /* Post-increment in expression */
        
        /* Complex expression to prevent simple optimization */
        checksum = (checksum << 3) | (checksum >> 29);
        
        asm volatile("" : : "r"(flat), "r"(checksum) : "memory");
    }
    
    sum += checksum;
    use_int(sum);
    return sum;
}

/* Test 5: Mixed pointer types and arithmetic */
void test5_mixed_pointers(char* buf, int* ints, size_t n) {
    char* cp = buf;
    int* ip = ints;
    int* ip_end = ints + n;
    
    /* Register pressure with different types */
    char c_acc = 0;
    int i_acc = 0;
    long long ll_acc = 0;
    
    /* Process integers with post-increment */
    while (ip < ip_end) {
        i_acc += *ip++;  /* Integer post-increment */
        
        /* Also process chars with different stride */
        if ((ip - ints) % 4 == 0) {
            c_acc += *cp++;  /* Char post-increment */
        }
        
        /* Mixed type calculations */
        ll_acc += (long long)i_acc * c_acc;
        
        /* Force register spilling */
        asm volatile("" : : "r"(ip), "r"(cp), "r"(i_acc), "r"(c_acc), "r"(ll_acc) : "memory");
    }
    
    /* Process remaining chars */
    char* cp_end = buf + n;
    while (cp < cp_end) {
        c_acc += *cp++;
        asm volatile("" : : "r"(cp) : "memory");
    }
    
    use_int(i_acc);
    use_char(c_acc);
    use_int((int)ll_acc);
}

/* Test 6: Pointer arithmetic that decomposes to reg+0 */
int test6_pointer_arithmetic(int* arr, size_t n, int stride) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Combined form that might decompose */
    while (p < end) {
        /* *(p += stride) - increment before access */
        p += stride;
        sum += *(p - stride);  /* Access previous location */
        
        /* Also try: sum += *(p++) with stride 1 */
        if (stride == 1) {
            sum += *p++;  /* This should hit the target pattern */
        }
        
        /* Complex live range */
        int* q = p - 1;
        sum += *q * 2;
        
        asm volatile("" : : "r"(p), "r"(q) : "memory");
    }
    
    /* Another pattern: array[i++] */
    int i = 0;
    int local_sum = 0;
    while (i < (int)n) {
        local_sum += arr[i++];  /* Index post-increment */
        
        /* Multiple live variables */
        int j = i * 2;
        int k = j + 1;
        local_sum += j - k;
        
        asm volatile("" : : "r"(i), "r"(local_sum) : "memory");
    }
    
    sum += local_sum;
    use_int(sum);
    return sum;
}

/* Opaque function implementations */
void use_int(int x) {
    global_checksum ^= x;
    asm volatile("" : : "r"(x) : "memory");
}

void use_ptr(void* p) {
    global_checksum += (int)((long)p & 0xFFFF);
    asm volatile("" : : "r"(p) : "memory");
}

void use_char(char c) {
    global_checksum += c;
    asm volatile("" : : "r"(c) : "memory");
}

void barrier(void) {
    asm volatile("" : : : "memory");
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const size_t N = 256;
    int int_array[N];
    char char_array[N];
    struct Data struct_array[N];
    int matrix[5][10];
    
    /* Initialize arrays with non-constant data */
    for (size_t i = 0; i < N; i++) {
        int_array[i] = (i * 37) & 0xFF;
        char_array[i] = (i * 13) & 0x7F;
        struct_array[i].value = i;
        struct_array[i].tag = i & 0xFF;
        struct_array[i].payload[0] = i * 2;
        struct_array[i].payload[1] = i * 3;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Use command-line argument to select tests */
    int test_mask = 0xFF;  /* Run all tests by default */
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    }
    
    /* Run selected tests */
    if (test_mask & 0x01) {
        int r1 = test1_sum_int_array(int_array, N);
        printf("Test1 result: %d\n", r1);
    }
    
    if (test_mask & 0x02) {
        char dest[N];
        test2_copy_string(dest, char_array, N);
        printf("Test2 completed\n");
    }
    
    if (test_mask & 0x04) {
        int r3 = test3_struct_array(struct_array, N / 4);
        printf("Test3 result: %d\n", r3);
    }
    
    if (test_mask & 0x08) {
        int r4 = test4_nested_loops(matrix, 5, 10);
        printf("Test4 result: %d\n", r4);
    }
    
    if (test_mask & 0x10) {
        test5_mixed_pointers(char_array, int_array, N);
        printf("Test5 completed\n");
    }
    
    if (test_mask & 0x20) {
        int r6 = test6_pointer_arithmetic(int_array, N, 1);
        printf("Test6 result: %d\n", r6);
    }
    
    /* Also run with stride != 1 */
    if (test_mask & 0x40) {
        int r6b = test6_pointer_arithmetic(int_array, N / 2, 2);
        printf("Test6b result: %d\n", r6b);
    }
    
    printf("Global checksum: %d\n", global_checksum);
    return global_checksum != 0 ? 0 : 1;
}
