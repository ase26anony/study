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

/* Test 1: Integer array summation with post-increment pointer */
int test1_int_postinc(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
    
    while (p < end) {
        /* Post-increment access - should generate (mem (plus (reg) (const_int 0))) */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 += sum; r2 += r1; r3 += r2; r4 += r3; r5 += r4;
        r6 += r5; r7 += r6; r8 += r7; r9 += r8; r10 += r9;
        
        /* Prevent elimination of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all pressure variables */
    sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    use_int(sum);
    return sum;
}

/* Test 2: String copy with dual post-increment */
void test2_char_postinc(char* dst, const char* src, int n) {
    char* d = dst;
    const char* s = src;
    int i = 0;
    
    /* Register pressure */
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd', c5 = 'e';
    
    do {
        /* Classic *dst++ = *src++ pattern */
        *d++ = *s++;
        i++;
        
        /* Use pressure variables */
        c1 = *d; c2 = *s; c3 = c1 + c2; c4 = c3 + i; c5 = c4 + *d;
        
        /* Opaque use of pointers */
        use_ptr(d);
        use_ptr((void*)s);
    } while (i < n);
    
    /* Ensure pointers are used */
    asm volatile("" : : "r"(d), "r"(s) : "memory");
    use_char(c1 + c2 + c3 + c4 + c5);
}

/* Simple struct for testing */
struct Point {
    int x;
    int y;
    int z;
};

/* Test 3: Struct array traversal with post-increment */
int test3_struct_postinc(struct Point* points, int n) {
    int total = 0;
    struct Point* ptr = points;
    int count = 0;
    
    /* Heavy register pressure */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int b1 = 6, b2 = 7, b3 = 8, b4 = 9, b5 = 10;
    
    while (count++ < n) {
        /* Access struct member via post-increment pointer */
        total += ptr->x + ptr->y + ptr->z;
        
        /* Post-increment the pointer */
        struct Point* old_ptr = ptr++;
        
        /* Complex calculations with pressure variables */
        a1 += old_ptr->x; a2 += old_ptr->y; a3 += old_ptr->z;
        b1 += a1; b2 += a2; b3 += a3; b4 += b1; b5 += b2;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(ptr), "r"(old_ptr) : "memory");
    }
    
    total += a1 + a2 + a3 + a4 + a5 + b1 + b2 + b3 + b4 + b5;
    use_int(total);
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_postinc(int* matrix, int rows, int cols) {
    int sum = 0;
    
    /* Multiple index variables for pressure */
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    
    for (i = 0; i < rows; i++) {
        int* row_ptr = matrix + i * cols;
        int* row_end = row_ptr + cols;
        
        /* Inner loop with pointer post-increment */
        while (row_ptr < row_end) {
            /* This should create the target RTL pattern */
            sum += *row_ptr++;
            
            /* Additional pressure */
            j += sum; k += j; l += k; m += l;
            
            /* Mix in some array indexing to create different patterns */
            if (row_ptr < row_end) {
                sum += matrix[i * cols + (row_ptr - matrix - i * cols)];
            }
        }
        
        /* Use pressure variables */
        asm volatile("" : : "r"(j), "r"(k), "r"(l), "r"(m) : "memory");
    }
    
    sum += j + k + l + m;
    return sum;
}

/* Test 5: Mixed pointer arithmetic with stride */
int test5_mixed_arithmetic(int* arr, int n, int stride) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create aliasing concerns */
    int* alias1 = arr + 1;
    int* alias2 = arr + 2;
    
    while (p < end) {
        /* Combined form that may decompose to base+offset */
        sum += *(p += stride);
        
        /* Use aliases to confuse optimizer */
        *alias1 = sum % 256;
        *alias2 = *alias1 + 1;
        
        /* Heavy register pressure */
        int t1 = *p, t2 = *alias1, t3 = *alias2;
        int t4 = t1 + t2, t5 = t3 + t4;
        sum += t4 + t5;
        
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(p), "r"(alias1), "r"(alias2) : "memory");
        
        /* Adjust for next iteration */
        p -= (stride - 1);
    }
    
    use_int(sum);
    return sum;
}

/* Test 6: Post-decrement pattern */
int test6_postdec(int* arr, int n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* Register pressure */
    int v[10];
    for (int i = 0; i < 10; i++) v[i] = i * i;
    
    while (p >= arr) {
        /* Post-decrement access */
        sum += *p--;
        
        /* Use pressure array */
        for (int i = 0; i < 10; i++) {
            v[i] += sum;
            sum += v[i] % 16;
        }
        
        /* Prevent elimination */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Consolidate pressure variables */
    for (int i = 0; i < 10; i++) sum += v[i];
    return sum;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int N = 256;
    int* int_arr = malloc(N * sizeof(int));
    char* char_arr = malloc(N * sizeof(char));
    struct Point* struct_arr = malloc(N * sizeof(struct Point));
    
    /* Initialize with non-constant data */
    for (int i = 0; i < N; i++) {
        int_arr[i] = (i * 37) % 101;
        char_arr[i] = 'A' + (i % 26);
        struct_arr[i].x = i;
        struct_arr[i].y = i * 2;
        struct_arr[i].z = i * 3;
    }
    
    int result = 0;
    
    /* Use command-line argument to select test */
    int test_num = (argc > 1) ? atoi(argv[1]) % 7 : 0;
    
    switch (test_num) {
        case 0:
            result = test1_int_postinc(int_arr, N);
            break;
        case 1:
            test2_char_postinc(char_arr, char_arr + N/2, N/2);
            result = char_arr[N/2];
            break;
        case 2:
            result = test3_struct_postinc(struct_arr, N/4);
            break;
        case 3:
            result = test4_nested_postinc(int_arr, 16, 16);
            break;
        case 4:
            result = test5_mixed_arithmetic(int_arr, N, 3);
            break;
        case 5:
            result = test6_postdec(int_arr, N);
            break;
        default:
            /* Run all tests */
            result = test1_int_postinc(int_arr, N);
            test2_char_postinc(char_arr, char_arr + N/2, N/2);
            result += test3_struct_postinc(struct_arr, N/4);
            result += test4_nested_postinc(int_arr, 16, 16);
            result += test5_mixed_arithmetic(int_arr, N, 3);
            result += test6_postdec(int_arr, N);
            break;
    }
    
    /* Update global to prevent elimination */
    global_checksum += result;
    
    /* Opaque use of result */
    use_int(result);
    
    free(int_arr);
    free(char_arr);
    free(struct_arr);
    
    printf("Result: %d\n", result);
    return global_checksum != 0 ? 0 : 1;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (long)p & 0xFF;
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += c;
}
