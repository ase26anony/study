/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_sum = 0;

/* Struct for testing pointer arithmetic */
struct Data {
    int value;
    char tag;
    float weight;
};

/* Pattern 1: Integer array sum with post-increment pointer */
int test_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many variables to increase register pressure */
        r1 += sum; r2 += r1; r3 += r2; r4 += r3; r5 += r4;
        r6 += r5; r7 += r6; r8 += r7; r9 += r8; r10 += r9;
        
        /* Prevent optimization with asm */
        asm volatile("" : : "r"(p), "r"(sum));
    }
    
    /* Use all pressure variables */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    use_ptr(p);
    return sum;
}

/* Pattern 2: String copy with post-increment */
void test_string_copy(char* dst, const char* src, int n) {
    int i = 0;
    
    /* Register pressure variables */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    int cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
    
    do {
        /* Classic post-increment copy pattern */
        *dst++ = *src++;
        i++;
        
        /* Use variables to maintain pressure */
        c1++; c2++; c3++; c4++;
        cnt1 += c1; cnt2 += c2; cnt3 += c3; cnt4 += c4;
        
        /* Opaque function call to prevent optimization */
        use_char(c1);
        
    } while (i < n && *(src-1) != '\0');
    
    /* Use all variables */
    dst[-1] = c1 + c2 + c3 + c4;
    use_int(cnt1 + cnt2 + cnt3 + cnt4);
}

/* Pattern 3: Struct array traversal with post-increment */
float test_struct_array(struct Data* arr, int n) {
    float total = 0.0f;
    struct Data* p = arr;
    
    /* High register pressure */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    
    for (int i = 0; i < n; i++) {
        /* Post-increment access to struct member */
        total += p++->value * p->weight;
        
        /* Complex calculations with pressure variables */
        f1 = f1 * 1.1f + total;
        f2 = f2 * 1.2f + f1;
        f3 = f3 * 1.3f + f2;
        f4 = f4 * 1.4f + f3;
        
        i1 += (int)f1; i2 += i1; i3 += i2; i4 += i3;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total));
    }
    
    /* Use all variables */
    total += f1 + f2 + f3 + f4 + i1 + i2 + i3 + i4;
    use_ptr(p);
    return total;
}

/* Pattern 4: Nested loops with array indexing and post-increment */
int test_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for pressure */
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    int* row_ptr;
    
    for (i = 0; i < rows; i++) {
        row_ptr = matrix[i];
        j = 0;
        
        /* Inner loop with post-increment */
        while (j < cols) {
            /* Access with post-increment on index */
            sum += row_ptr[j++];
            
            /* Additional operations to prevent optimization */
            k = sum * 2;
            l = k + j;
            m = l * 3;
            
            /* Use asm to keep variables live */
            asm volatile("" : : "r"(row_ptr), "r"(j), "r"(sum));
        }
        
        /* Use computed values */
        sum += k + l + m;
        use_int(sum);
    }
    
    return sum;
}

/* Pattern 5: Mixed pointer arithmetic with stride */
int test_mixed_arithmetic(int* arr, int n, int stride) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Extreme register pressure */
    int v[20];
    for (int i = 0; i < 20; i++) v[i] = i * 2;
    
    /* Loop with combined pointer arithmetic */
    while (p < end) {
        /* Different forms of pointer access */
        sum += *p;          /* Direct access */
        p += stride;        /* Pointer increment with stride */
        
        if (p >= end) break;
        
        sum += *(p - 1);    /* Offset access */
        
        /* Use all pressure variables */
        for (int i = 0; i < 20; i++) {
            v[i] += sum + i;
            asm volatile("" : : "r"(v[i]));
        }
        
        /* Opaque call */
        use_ptr(p);
    }
    
    /* Final computation with all variables */
    for (int i = 0; i < 20; i++) sum += v[i];
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1000;
    
    /* Integer array */
    int* int_arr = malloc(ARRAY_SIZE * sizeof(int));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i % 100;
    }
    
    /* String data */
    char* src_str = malloc(ARRAY_SIZE);
    char* dst_str = malloc(ARRAY_SIZE);
    memset(src_str, 'A', ARRAY_SIZE - 1);
    src_str[ARRAY_SIZE - 1] = '\0';
    
    /* Struct array */
    struct Data* struct_arr = malloc(ARRAY_SIZE * sizeof(struct Data));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        struct_arr[i].value = i;
        struct_arr[i].tag = 'A' + (i % 26);
        struct_arr[i].weight = i * 0.1f;
    }
    
    /* Matrix data */
    int rows = 100, cols = 10;
    int** matrix = malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = i * cols + j;
        }
    }
    
    /* Select test based on command line argument */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    switch (test_num) {
        case 1:
            result = test_int_array_sum(int_arr, ARRAY_SIZE);
            break;
        case 2:
            test_string_copy(dst_str, src_str, ARRAY_SIZE);
            result = dst_str[0];
            break;
        case 3:
            result = (int)test_struct_array(struct_arr, ARRAY_SIZE);
            break;
        case 4:
            result = test_nested_loops(matrix, rows, cols);
            break;
        case 5:
            result = test_mixed_arithmetic(int_arr, ARRAY_SIZE, 2);
            break;
        default:
            /* Run all tests */
            result = test_int_array_sum(int_arr, ARRAY_SIZE);
            test_string_copy(dst_str, src_str, ARRAY_SIZE);
            result += (int)test_struct_array(struct_arr, ARRAY_SIZE);
            result += test_nested_loops(matrix, rows, cols);
            result += test_mixed_arithmetic(int_arr, ARRAY_SIZE, 2);
            break;
    }
    
    /* Update global to prevent elimination */
    global_sum = result;
    
    /* Cleanup */
    free(int_arr);
    free(src_str);
    free(dst_str);
    free(struct_arr);
    for (int i = 0; i < rows; i++) free(matrix[i]);
    free(matrix);
    
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
    global_sum += (int)c;
}
