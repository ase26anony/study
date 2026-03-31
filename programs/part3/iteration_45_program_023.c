/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct for testing pointer arithmetic */
struct Data {
    int value;
    char tag;
    float weight;
};

/* Pattern 1: Integer array summation with post-increment pointer */
int test_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 = sum * 2;
        r2 = r1 + *p;
        r3 = r2 - sum;
        r4 = r3 ^ r1;
        r5 = r4 | r2;
        r6 = r5 & r3;
        r7 = r6 << 2;
        r8 = r7 >> 1;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(r1), "r"(r2), "r"(r3), 
                      "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8));
    }
    
    /* Use opaque function to prevent elimination */
    use_ptr(p);
    return sum + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

/* Pattern 2: String copy with post-increment */
void test_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
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
        t1 = c1 + c2;
        t2 = t1 * i;
        t3 = t2 ^ (int)c1;
        t4 = t3 | (int)c2;
        
        /* Force pointer usage */
        asm volatile("" : : "r"(d), "r"(s), "r"(t1), "r"(t2));
        
        i++;
    } while (i < n);
    
    /* Prevent tail optimization */
    use_ptr(d);
    use_ptr(s);
}

/* Pattern 3: Struct array traversal with post-increment */
int test_struct_array(struct Data* arr, int n) {
    int total = 0;
    struct Data* p = arr;
    
    /* Heavy register pressure */
    float f1 = 0.0f, f2 = 0.0f, f3 = 0.0f;
    int i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0;
    char ch1, ch2, ch3;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->value;
        
        /* Multiple operations on struct fields */
        f1 += p->weight;
        ch1 = p->tag;
        i1 += (int)ch1;
        
        /* Post-increment pointer */
        p++;
        
        /* More register pressure */
        i2 = total ^ i1;
        i3 = i2 * (int)f1;
        i4 = i3 + p->value;  /* Next element */
        i5 = i4 - i2;
        
        /* Force all variables to be live */
        asm volatile("" : : "r"(p), "r"(f1), "r"(i1), "r"(i2), 
                      "r"(i3), "r"(i4), "r"(i5), "r"(ch1));
    }
    
    use_ptr(p);
    return total + (int)f1 + i1 + i2 + i3 + i4 + i5;
}

/* Pattern 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    int* row_ptr;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        j = 0;
        
        /* Inner loop with pointer arithmetic */
        while (j < cols) {
            /* Combined form that may decompose to base+offset */
            sum += *(row_ptr + j);
            
            /* Post-increment the index */
            j++;
            
            /* Additional operations */
            k = sum ^ j;
            l = k * i;
            m = l + *row_ptr;
            
            /* Force register usage */
            asm volatile("" : : "r"(row_ptr), "r"(j), "r"(k), 
                          "r"(l), "r"(m));
        }
        
        /* Alternative: explicit post-increment in expression */
        for (l = 0; l < cols; l++) {
            sum += *row_ptr++;
            asm volatile("" : : "r"(row_ptr));
        }
    }
    
    return sum + k + l + m;
}

/* Pattern 5: Mixed pointer types with stride */
int test_mixed_pointers(char* data, int size) {
    int* int_ptr = (int*)data;
    char* char_ptr = data;
    short* short_ptr = (short*)data;
    
    int int_sum = 0;
    char char_sum = 0;
    short short_sum = 0;
    
    /* Register pressure cluster */
    int r[10] = {0};
    
    for (int i = 0; i < size / 4; i++) {
        /* Different pointer types with post-increment */
        int_sum += *int_ptr++;
        
        if (i < size) {
            char_sum += *char_ptr++;
        }
        
        if (i < size / 2) {
            short_sum += *short_ptr++;
        }
        
        /* Complex calculations to use many registers */
        for (int j = 0; j < 10; j++) {
            r[j] += int_sum * j + char_sum + short_sum;
            asm volatile("" : : "r"(r[j]));
        }
        
        /* Force pointer values to be used */
        asm volatile("" : : "r"(int_ptr), "r"(char_ptr), "r"(short_ptr));
    }
    
    /* Combine results in non-trivial way */
    int total = int_sum;
    for (int j = 0; j < 10; j++) {
        total ^= r[j];
    }
    
    return total + char_sum + short_sum;
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
    int** matrix = malloc(MATRIX_ROWS * sizeof(int*));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 37) & 0xFF;
        char_array[i] = (i * 13) & 0x7F;
        struct_array[i].value = i;
        struct_array[i].tag = (i % 26) + 'A';
        struct_array[i].weight = (float)i / 100.0f;
    }
    
    for (int i = 0; i < MATRIX_ROWS; i++) {
        matrix[i] = malloc(MATRIX_COLS * sizeof(int));
        for (int j = 0; j < MATRIX_COLS; j++) {
            matrix[i][j] = (i * MATRIX_COLS + j) * 7;
        }
    }
    
    /* Select test based on command line to prevent constant folding */
    int test_num = (argc > 1) ? atoi(argv[1]) % 5 : 0;
    int result = 0;
    
    switch (test_num) {
        case 0:
            result = test_int_array_sum(int_array, ARRAY_SIZE);
            break;
        case 1:
            test_string_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result = char_array[0] + char_array[ARRAY_SIZE-1];
            break;
        case 2:
            result = test_struct_array(struct_array, ARRAY_SIZE);
            break;
        case 3:
            result = test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
            break;
        case 4:
            result = test_mixed_pointers(char_array, ARRAY_SIZE);
            break;
    }
    
    /* Update global to prevent elimination */
    global_checksum ^= result;
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_ROWS; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(char_array);
    free(struct_array);
    
    /* Print result to ensure code runs */
    printf("Result: %d\n", result);
    printf("Checksum: %d\n", global_checksum);
    
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    if (p) global_checksum += 1;
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += (int)c;
}
