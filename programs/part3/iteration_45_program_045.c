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

/* Global volatile to prevent dead code elimination */
volatile int global_sum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int id;
    float weight;
};

/* Prevent inlining to preserve patterns */
__attribute__((noinline))
int test_post_increment_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* Access memory with post-increment */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 = sum * 2;
        r2 = r1 + 3;
        r3 = r2 - r1;
        r4 = r3 * r2;
        r5 = r4 / (r1 + 1);
        r6 = r5 ^ r4;
        r7 = r6 | r3;
        r8 = r7 & r2;
        r9 = r8 << 2;
        r10 = r9 >> 1;
        
        /* Make pointer appear used */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all pressure variables to prevent elimination */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Opaque function call */
    use_int(sum);
    
    return sum;
}

__attribute__((noinline))
void test_char_array_copy(char* dst, const char* src, size_t len) {
    /* Classic post-increment copy pattern */
    const char* s = src;
    char* d = dst;
    
    /* Register pressure variables */
    char c1, c2, c3, c4, c5;
    int i1 = 0, i2 = 0, i3 = 0;
    
    size_t i = 0;
    do {
        /* Post-increment memory access */
        *d++ = *s++;
        
        /* Additional operations to prevent optimization */
        c1 = *s;
        c2 = c1 + 1;
        c3 = c2 * 2;
        c4 = c3 - c1;
        c5 = c4 ^ c2;
        
        i1 += c1;
        i2 += c2;
        i3 += c3;
        
        /* Barrier to prevent reordering */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
        
        i++;
    } while (i < len);
    
    /* Use variables */
    use_char(c5);
    use_int(i1 + i2 + i3);
}

__attribute__((noinline))
int test_struct_array_traversal(struct Data* data, int count) {
    int total = 0;
    struct Data* p = data;
    
    /* High register pressure */
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;
    float f1 = 0.0f, f2 = 0.0f;
    
    for (int i = 0; i < count; i++) {
        /* Access struct member with post-increment */
        total += p->value;
        
        /* Complex expression to use multiple registers */
        v1 = p->id * 2;
        v2 = v1 + p->value;
        v3 = v2 - p->id;
        v4 = v3 * v1;
        v5 = v4 / (p->value + 1);
        
        f1 += p->weight;
        f2 = f1 * 2.0f;
        
        /* Post-increment pointer */
        p++;
        
        /* Make pointer appear used */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    total += v1 + v2 + v3 + v4 + v5 + (int)f1 + (int)f2;
    return total;
}

__attribute__((noinline))
int test_nested_loops_with_index(int* matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l, m;
    int t1 = 0, t2 = 0, t3 = 0;
    
    for (i = 0; i < rows; i++) {
        int* row_ptr = matrix + i * cols;
        int* row_end = row_ptr + cols;
        
        /* Inner loop with pointer arithmetic */
        while (row_ptr < row_end) {
            /* Access with post-increment */
            sum += *row_ptr++;
            
            /* Additional computations */
            t1 = sum * i;
            t2 = t1 + j;
            t3 = t2 - sum;
            
            /* Nested index updates */
            for (k = 0; k < 2; k++) {
                for (l = 0; l < 2; l++) {
                    m = k + l;
                    t1 += m;
                }
            }
            
            /* Barrier */
            asm volatile("" : : "r"(row_ptr) : "memory");
        }
        
        /* Update outer loop variables */
        j = i * 2;
    }
    
    sum += t1 + t2 + t3 + j;
    return sum;
}

__attribute__((noinline))
int test_mixed_pointer_arithmetic(int* arr, int n, int stride) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Multiple pointer variables */
    int* q = arr;
    int* r = arr + n/2;
    
    /* High register pressure */
    register int reg1 asm("r0") = 0;
    register int reg2 asm("r1") = 0;
    register int reg3 asm("r2") = 0;
    
    while (p < end) {
        /* Combined pointer arithmetic that may decompose to base+0 */
        int* next = p + stride;
        
        /* Access current, then update */
        sum += *p;
        
        /* Multiple update forms */
        p = next;
        
        /* Also use other pointers */
        reg1 += *q++;
        reg2 += *r--;
        
        /* Complex expression */
        reg3 = (reg1 * reg2) + (p - arr);
        
        /* Opaque use */
        use_ptr(p);
        use_ptr(q);
        use_ptr(r);
        
        /* Memory barrier */
        barrier();
    }
    
    sum += reg1 + reg2 + reg3;
    return sum;
}

/* Dummy implementations of opaque functions */
void use_int(int x) {
    global_sum += x;
}

void use_ptr(void* p) {
    global_sum += (int)((size_t)p & 0xFF);
}

void use_char(char c) {
    global_sum += c;
}

void barrier(void) {
    asm volatile("" : : : "memory");
}

int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 32;
    const int MATRIX_COLS = 32;
    
    /* Allocate and initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_array1 = (char*)malloc(ARRAY_SIZE * sizeof(char));
    char* char_array2 = (char*)malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = (struct Data*)malloc(ARRAY_SIZE * sizeof(struct Data));
    int* matrix = (int*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 97;
        char_array1[i] = 'A' + (i % 26);
        struct_array[i].value = i * 2;
        struct_array[i].id = i;
        struct_array[i].weight = i * 0.5f;
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = (i * 7) % 113;
    }
    
    int result = 0;
    
    /* Use command line argument to select path */
    int test_case = (argc > 1) ? atoi(argv[1]) % 5 : 0;
    
    switch (test_case) {
        case 0:
            result = test_post_increment_sum(int_array, ARRAY_SIZE);
            break;
        case 1:
            test_char_array_copy(char_array2, char_array1, ARRAY_SIZE);
            result = char_array2[0] + char_array2[ARRAY_SIZE-1];
            break;
        case 2:
            result = test_struct_array_traversal(struct_array, ARRAY_SIZE);
            break;
        case 3:
            result = test_nested_loops_with_index(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            result = test_mixed_pointer_arithmetic(int_array, ARRAY_SIZE, 4);
            break;
    }
    
    /* Use result to prevent elimination */
    global_sum += result;
    
    /* Print to ensure code runs */
    printf("Result: %d, Global sum: %d\n", result, global_sum);
    
    /* Cleanup */
    free(int_array);
    free(char_array1);
    free(char_array2);
    free(struct_array);
    free(matrix);
    
    return 0;
}
