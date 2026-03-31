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
extern int get_value(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Struct to create complex memory accesses */
struct Data {
    int value;
    char tag;
    int payload[2];
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_sum_int_array(const int* arr, size_t n) {
    const int* p = arr;
    int sum = 0;
    
    /* Create register pressure with many live variables */
    int r0 = get_value(), r1 = get_value(), r2 = get_value();
    int r3 = get_value(), r4 = get_value(), r5 = get_value();
    
    /* Loop with post-increment pointer access */
    for (size_t i = 0; i < n; ++i) {
        /* Pattern: *p++ where p is modified after fetch */
        sum += *p++;
        
        /* Use all register-pressure variables to keep them live */
        r0 += sum; r1 ^= r0; r2 += r1; r3 ^= r2;
        r4 += r3; r5 ^= r4;
        
        /* Prevent optimization of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Consume all variables to prevent elimination */
    global_sink += r0 + r1 + r2 + r3 + r4 + r5;
    return sum;
}

/* Test 2: String copy with dual post-increment pointers */
void test2_copy_string(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    
    /* Register pressure variables */
    int t0 = 0, t1 = 0, t2 = 0, t3 = 0;
    
    /* Classic *dst++ = *src++ pattern */
    for (size_t i = 0; i < n; ++i) {
        *d++ = *s++;
        
        /* Complex use of variables to maintain pressure */
        t0 += *s; t1 += t0; t2 ^= t1; t3 += t2;
        
        /* Make pointers appear used */
        asm volatile("" : : "r"(d), "r"(s) : "memory");
    }
    
    /* Opaque function call to prevent aliasing analysis */
    use_ptr(dst);
    use_ptr(src);
    global_sink += t0 + t1 + t2 + t3;
}

/* Test 3: Struct array traversal with post-increment */
int test3_sum_struct_array(const struct Data* arr, size_t n) {
    const struct Data* p = arr;
    int sum = 0;
    
    /* Even more register pressure */
    int a = get_value(), b = get_value(), c = get_value();
    int d = get_value(), e = get_value(), f = get_value();
    int g = get_value(), h = get_value();
    
    /* Access struct member via post-increment pointer */
    for (size_t i = 0; i < n; ++i) {
        sum += p->value;
        p++;  /* Post-increment after access */
        
        /* Complex dependency chain to keep variables live */
        a += sum; b ^= a; c += b; d ^= c;
        e += d; f ^= e; g += f; h ^= g;
        
        /* Force pointer to stay in register */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    global_sink += a + b + c + d + e + f + g + h;
    return sum;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int matrix[][10], size_t rows) {
    int sum = 0;
    
    /* High register pressure */
    int v[8];
    for (int i = 0; i < 8; ++i) v[i] = get_value();
    
    for (size_t i = 0; i < rows; ++i) {
        int* row = matrix[i];
        int idx = 0;
        
        /* Inner loop with combined pointer arithmetic */
        for (size_t j = 0; j < 10; ++j) {
            /* Pattern: *(row + idx++) - decomposes to base+offset */
            sum += *(row + idx++);
            
            /* Update pressure variables */
            for (int k = 0; k < 8; ++k) {
                v[k] += sum + k;
            }
            
            /* Prevent optimization */
            asm volatile("" : : "r"(row), "r"(idx) : "memory");
        }
    }
    
    int total_v = 0;
    for (int i = 0; i < 8; ++i) total_v += v[i];
    global_sink += total_v;
    return sum;
}

/* Test 5: Mixed pointer types and stride access */
int test5_mixed_pointers(void* base, size_t n) {
    char* cptr = (char*)base;
    int* iptr = (int*)base;
    int sum = 0;
    
    /* Maximum register pressure */
    register int r0 asm("r0") = get_value();
    register int r1 asm("r1") = get_value();
    register int r2 asm("r2") = get_value();
    int r3 = get_value(), r4 = get_value(), r5 = get_value();
    int r6 = get_value(), r7 = get_value(), r8 = get_value();
    int r9 = get_value(), r10 = get_value();
    
    /* Access with different pointer types and strides */
    for (size_t i = 0; i < n; ++i) {
        /* Char pointer with post-increment */
        sum += *cptr++;
        
        /* Int pointer with stride (may decompose to base+0) */
        if (i % 2 == 0) {
            sum += *iptr;
            iptr += 2;  /* Stride of 2 */
        }
        
        /* Complex use of all pressure variables */
        r0 += sum; r1 ^= r0; r2 += r1; r3 ^= r2;
        r4 += r3; r5 ^= r4; r6 += r5; r7 ^= r6;
        r8 += r7; r9 ^= r8; r10 += r9;
        
        /* Force all pointers to be considered live */
        asm volatile("" : : "r"(cptr), "r"(iptr), "r"(r0), "r"(r1), "r"(r2) : "memory");
    }
    
    global_sink += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    return sum;
}

/* Test 6: do-while loop with post-decrement */
int test6_post_decrement(int* arr, size_t n) {
    int* p = arr + n - 1;
    int sum = 0;
    size_t count = n;
    
    /* Register pressure */
    int x[12];
    for (int i = 0; i < 12; ++i) x[i] = get_value();
    
    /* do-while with post-decrement */
    do {
        sum += *p--;
        
        /* Update all pressure variables */
        for (int i = 0; i < 12; ++i) {
            x[i] += sum + i;
        }
        
        asm volatile("" : : "r"(p) : "memory");
    } while (--count > 0);
    
    int total_x = 0;
    for (int i = 0; i < 12; ++i) total_x += x[i];
    global_sink += total_x;
    return sum;
}

/* Main function with command-line control */
int main(int argc, char* argv[]) {
    /* Initialize test data */
    int int_array[100];
    char char_array[100];
    struct Data struct_array[50];
    int matrix[5][10];
    
    /* Fill with pseudo-random data using argc as seed */
    for (int i = 0; i < 100; ++i) {
        int_array[i] = (i * 37 + argc) & 0xFF;
        char_array[i] = (i * 13 + argc) & 0x7F;
    }
    
    for (int i = 0; i < 50; ++i) {
        struct_array[i].value = (i * 29 + argc) & 0xFF;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].payload[0] = i;
        struct_array[i].payload[1] = i * 2;
    }
    
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 10; ++j) {
            matrix[i][j] = (i * 10 + j + argc) & 0xFF;
        }
    }
    
    int result = 0;
    
    /* Select tests based on command line to prevent constant folding */
    if (argc > 1) {
        int test_num = atoi(argv[1]) % 7;
        
        switch (test_num) {
            case 0:
                result = test1_sum_int_array(int_array, 100);
                break;
            case 1:
                test2_copy_string(char_array, char_array + 50, 50);
                result = char_array[25];
                break;
            case 2:
                result = test3_sum_struct_array(struct_array, 50);
                break;
            case 3:
                result = test4_nested_loops(matrix, 5);
                break;
            case 4:
                result = test5_mixed_pointers(int_array, 100);
                break;
            case 5:
                result = test6_post_decrement(int_array, 100);
                break;
            default:
                /* Run all tests */
                result = test1_sum_int_array(int_array, 100);
                test2_copy_string(char_array, char_array + 50, 50);
                result += test3_sum_struct_array(struct_array, 50);
                result += test4_nested_loops(matrix, 5);
                result += test5_mixed_pointers(int_array, 100);
                result += test6_post_decrement(int_array, 100);
                break;
        }
    } else {
        /* Default: run all tests */
        result = test1_sum_int_array(int_array, 100);
        test2_copy_string(char_array, char_array + 50, 50);
        result += test3_sum_struct_array(struct_array, 50);
        result += test4_nested_loops(matrix, 5);
        result += test5_mixed_pointers(int_array, 100);
        result += test6_post_decrement(int_array, 100);
    }
    
    printf("Result: %d (global_sink: %d)\n", result, global_sink);
    return 0;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_sink += x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_sink += (int)((size_t)p & 0xFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_sink += c;
}

int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}
