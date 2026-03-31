/* test-auto-inc-dec.c
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
volatile int global_check = 0;

/* Struct to create complex memory accesses */
struct Data {
    int value;
    char tag;
    int payload[2];
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_int_array_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    while (p < end) {
        /* Post-increment access - target pattern */
        sum += *p++;
        
        /* Use many registers to create pressure */
        r1 += sum; r2 += r1; r3 += r2; r4 += r3;
        r5 += r4; r6 += r5; r7 += r6; r8 += r7;
        r9 += r8; r10 += r9;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(sum));
    }
    
    /* Consume register variables */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Opaque use to prevent elimination */
    use_ptr(p);
    return sum;
}

/* Test 2: String copy with post-increment */
void test2_string_copy(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    int i = 0;
    
    /* Register pressure */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    
    do {
        /* Classic post-increment copy pattern */
        *d++ = *s++;
        
        /* Use registers */
        c1 = *s; c2 = c1 + 1; c3 = c2 + 1; c4 = c3 + 1;
        t1 += c1; t2 += c2; t3 += c3; t4 += c4;
        
        /* Barrier to prevent reordering */
        asm volatile("" : : "r"(d), "r"(s));
        
        i++;
    } while (i < n);
    
    /* Opaque uses */
    use_char(c1);
    use_int(t1 + t2 + t3 + t4);
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array(struct Data* arr, int n) {
    int total = 0;
    struct Data* p = arr;
    
    /* Heavy register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->value;
        
        /* Complex addressing that may decompose to base+0 */
        acc1 += p->payload[0];
        acc2 += p->payload[1];
        
        /* Post-increment pointer */
        p++;
        
        /* More register usage */
        acc3 += acc1; acc4 += acc2; acc5 += acc3;
        acc6 += acc4; acc7 += acc5; acc8 += acc6;
        acc9 += acc7; acc10 += acc8;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total));
    }
    
    total += acc1 + acc2 + acc3 + acc4 + acc5 + 
             acc6 + acc7 + acc8 + acc9 + acc10;
    
    use_ptr(p);
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int** matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        
        for (j = 0; j < cols; j++) {
            /* Array access with pointer post-increment */
            sum += *p++;
            
            /* Additional computations using registers */
            tmp1 = sum * i;
            tmp2 = tmp1 + j;
            tmp3 = tmp2 * 3;
            tmp4 = tmp3 / 2;
            
            k += tmp1; l += tmp2; m += tmp3;
            
            /* Barrier */
            asm volatile("" : : "r"(p), "r"(sum));
        }
        
        /* Use computed values */
        sum += k + l + m + tmp4;
    }
    
    return sum;
}

/* Test 5: Mixed pointer types and stride access */
int test5_mixed_pointers(char* data, int size, int stride) {
    int sum = 0;
    char* p = data;
    int* ip = (int*)data;
    
    /* Register pressure variables */
    int a = 0, b = 0, c = 0, d = 0, e = 0;
    int f = 0, g = 0, h = 0, ii = 0, j = 0;
    
    /* Loop with pointer arithmetic that may create (plus (reg) (const_int 0)) */
    for (int i = 0; i < size; i += stride) {
        /* Different pointer access patterns */
        sum += *p;          /* char access */
        p += stride;        /* Pointer increment */
        
        sum += *ip;         /* int access */
        ip = (int*)((char*)ip + stride);  /* Pointer with cast */
        
        /* Register pressure calculations */
        a = sum * 2; b = a + i; c = b * 3;
        d = c / 2; e = d + stride;
        f = e * a; g = f - b; h = g + c;
        ii = h * 2; j = ii / 3;
        
        /* Use all registers */
        sum += a + b + c + d + e + f + g + h + ii + j;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(ip), "r"(sum));
    }
    
    return sum;
}

/* Test 6: Do-while loop with post-decrement */
int test6_post_decrement(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    int count = n;
    
    /* Register pressure */
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    
    do {
        /* Post-decrement access */
        sum += *p--;
        
        /* Register calculations */
        x1 = sum * x1;
        x2 = x1 + x2;
        x3 = x2 * x3;
        x4 = x3 - x4;
        x5 = x4 + x5;
        
        count--;
        
        /* Barrier */
        asm volatile("" : : "r"(p), "r"(sum));
    } while (count > 0);
    
    sum += x1 + x2 + x3 + x4 + x5;
    return sum;
}

/* Dummy implementations of opaque functions */
void use_int(int x) {
    global_sum += x;
}

void use_ptr(void* p) {
    global_check ^= (int)(long)p;
}

void use_char(char c) {
    global_sum += c;
}

void barrier(void) {
    asm volatile("" : : : "memory");
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    int test_to_run = 0;
    
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Initialize test data */
    const int ARRAY_SIZE = 1000;
    int* int_array = malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = malloc(ARRAY_SIZE * sizeof(struct Data));
    int** matrix = malloc(10 * sizeof(int*));
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i % 100;
        char_array[i] = 'A' + (i % 26);
        struct_array[i].value = i;
        struct_array[i].tag = 'X';
        struct_array[i].payload[0] = i * 2;
        struct_array[i].payload[1] = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        matrix[i] = malloc(10 * sizeof(int));
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    int result = 0;
    
    /* Select test based on input */
    switch (test_to_run % 7) {
        case 0:
            result = test1_int_array_sum(int_array, ARRAY_SIZE);
            break;
        case 1:
            test2_string_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result = char_array[0];
            break;
        case 2:
            result = test3_struct_array(struct_array, ARRAY_SIZE);
            break;
        case 3:
            result = test4_nested_loops(matrix, 10, 10);
            break;
        case 4:
            result = test5_mixed_pointers(char_array, ARRAY_SIZE, 4);
            break;
        case 5:
            result = test6_post_decrement(int_array, ARRAY_SIZE);
            break;
        case 6:
            /* Run all tests */
            result = test1_int_array_sum(int_array, ARRAY_SIZE);
            test2_string_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result += test3_struct_array(struct_array, ARRAY_SIZE);
            result += test4_nested_loops(matrix, 10, 10);
            result += test5_mixed_pointers(char_array, ARRAY_SIZE, 4);
            result += test6_post_decrement(int_array, ARRAY_SIZE);
            break;
    }
    
    /* Use result to prevent elimination */
    global_sum = result;
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(int_array);
    free(char_array);
    free(struct_array);
    
    printf("Result: %d\n", result);
    return 0;
}
