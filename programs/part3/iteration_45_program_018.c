/* test_auto_inc_dec.c - Program to trigger auto-increment/decrement optimization patterns */

#include <stddef.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern int get_seed(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int id;
    float weight;
};

/* Test 1: Integer array summation with post-increment pointer */
int test_int_array_sum(int* arr, int size) {
    int sum = 0;
    int* p = arr;
    int* end = arr + size;
    
    /* Create register pressure with many local variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* The key pattern: memory access with post-increment */
        sum += *p++;
        
        /* Use the temporary variables to create register pressure */
        temp1 += sum & 0xFF;
        temp2 += sum >> 8;
        temp3 += temp1 * 2;
        temp4 += temp2 / 3;
        temp5 = temp3 ^ temp4;
        temp6 = temp5 | sum;
        temp7 = temp6 & 0xFFFF;
        temp8 = temp7 + 1;
        
        /* Inline asm to prevent optimization */
        asm volatile("" : : "r"(p), "r"(sum), "r"(temp1), "r"(temp2));
    }
    
    /* Mix all temps to force them to be live */
    sum += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
    
    /* Use opaque function to prevent elimination */
    use_int(sum);
    
    return sum;
}

/* Test 2: String copy with post-increment on both source and dest */
void test_string_copy(char* dest, const char* src, size_t len) {
    char* d = dest;
    const char* s = src;
    const char* end = src + len;
    
    /* Register pressure variables */
    char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    int count = 0;
    
    /* Classic post-increment copy pattern */
    while (s < end) {
        /* The key pattern: dual post-increment memory access */
        *d++ = *s++;
        
        /* Additional operations to create register pressure */
        c1 = *s;
        c2 = c1 ^ 0x55;
        c3 = c2 + count;
        c4 = c3 & 0x7F;
        count++;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(d), "r"(s), "r"(c1));
    }
    
    /* Force variables to be used */
    use_char(c1 + c2 + c3 + c4);
    use_ptr(dest);
}

/* Test 3: Struct array traversal with post-increment */
int test_struct_array(struct Data* arr, int count) {
    int total = 0;
    struct Data* p = arr;
    struct Data* end = arr + count;
    
    /* High register pressure */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;
    float f1 = 0.0f, f2 = 0.0f;
    
    /* Loop with struct pointer post-increment */
    for (; p < end; p++) {
        /* Access struct member with pointer that was just incremented */
        total += p->value;
        
        /* More operations for register pressure */
        v1 += p->id;
        v2 = p->tag;
        v3 = v1 * v2;
        v4 = v3 + total;
        v5 = v4 & 0xFFF;
        f1 += p->weight;
        f2 = f1 * 2.0f;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total), "r"(v1));
    }
    
    total += (int)(f1 + f2) + v1 + v2 + v3 + v4 + v5;
    use_int(total);
    
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int* matrix, int rows, int cols) {
    int sum = 0;
    
    /* Many local variables for register pressure */
    int i, j, k, l, m, n;
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    
    for (i = 0; i < rows; i++) {
        int* row_ptr = matrix + i * cols;
        int* row_end = row_ptr + cols;
        
        /* Inner loop with pointer post-increment */
        for (j = 0; row_ptr < row_end; ) {
            /* Combined forms that may decompose to base+offset */
            sum += *(row_ptr + j);
            j++;
            
            /* Alternative pattern using pointer arithmetic */
            t1 += *(row_ptr += 1);  /* This may create plus(const_int 0) pattern */
            
            /* More register pressure */
            k = i * j;
            l = k + sum;
            m = l ^ t1;
            n = m & 0xFF;
            t2 += n;
            t3 = t2 * 2;
            t4 = t3 - 1;
            
            asm volatile("" : : "r"(row_ptr), "r"(sum), "r"(t1));
        }
        
        /* Mix variables to keep them live */
        sum += t1 + t2 + t3 + t4;
    }
    
    return sum;
}

/* Test 5: Mixed pointer types and arithmetic */
int test_mixed_pointers(void* base, int iterations) {
    char* cptr = (char*)base;
    int* iptr = (int*)base;
    struct Data* sptr = (struct Data*)base;
    
    int result = 0;
    
    /* Create complex addressing patterns */
    for (int i = 0; i < iterations; i++) {
        /* Different pointer types with post-increment */
        char c = *cptr++;
        int val = *iptr++;
        int id = sptr++->id;
        
        /* Combined operations */
        result += c + val + id;
        
        /* Pointer arithmetic that may create base+0 patterns */
        int* p = iptr + 0;  /* Explicit plus with zero */
        result += *p;
        
        /* More complex addressing */
        result += *(cptr += 1) - 1;  /* Post-increment in expression */
        
        /* Register pressure */
        int temp = result;
        for (int j = 0; j < 4; j++) {
            temp = (temp << 3) | (temp >> 5);
            asm volatile("" : : "r"(temp));
        }
        result = temp;
    }
    
    return result;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int INT_ARRAY_SIZE = 1000;
    const int STRING_LENGTH = 500;
    const int STRUCT_COUNT = 200;
    const int MATRIX_SIZE = 50;
    
    static int int_array[1000];
    static char src_string[500];
    static char dest_string[500];
    static struct Data struct_array[200];
    static int matrix[50][50];
    
    /* Initialize with non-constant data */
    int seed = get_seed();
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        int_array[i] = (i * seed) & 0xFFF;
    }
    
    for (int i = 0; i < STRING_LENGTH; i++) {
        src_string[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = i * 3;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].id = i + seed;
        struct_array[i].weight = i * 0.5f;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = (i * j + seed) & 0xFF;
        }
    }
    
    /* Select tests based on command line to prevent constant folding */
    int test_mask = 0;
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    } else {
        test_mask = 0x1F; /* Run all tests */
    }
    
    int final_result = 0;
    
    if (test_mask & 0x01) {
        final_result ^= test_int_array_sum(int_array, INT_ARRAY_SIZE);
    }
    
    if (test_mask & 0x02) {
        test_string_copy(dest_string, src_string, STRING_LENGTH);
        /* Use result to prevent elimination */
        for (int i = 0; i < 10; i++) {
            final_result += dest_string[i];
        }
    }
    
    if (test_mask & 0x04) {
        final_result ^= test_struct_array(struct_array, STRUCT_COUNT);
    }
    
    if (test_mask & 0x08) {
        final_result ^= test_nested_loops(&matrix[0][0], MATRIX_SIZE, MATRIX_SIZE);
    }
    
    if (test_mask & 0x10) {
        final_result ^= test_mixed_pointers(int_array, 100);
    }
    
    /* Update global volatile to ensure all computations are used */
    global_checksum = final_result;
    
    return final_result & 0xFF;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (int)((size_t)p & 0xFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += c;
}

int __attribute__((noinline)) get_seed(void) {
    return 42; /* Not truly random, but prevents compile-time computation */
}
