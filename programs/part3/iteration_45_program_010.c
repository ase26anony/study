/* auto-inc-dec-test.c
 * Designed to trigger find_inc(true) path in GCC's auto-inc-dec optimization
 * Specifically targets lines 1352-1358 of auto-inc-dec.cc
 */

#include <stddef.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern int get_value(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int payload[2];
};

/* Test 1: Integer array summation with post-increment pointer */
int test1_sum_int_array(const int* arr, size_t n) {
    int sum = 0;
    const int* p = arr;
    const int* end = arr + n;
    
    /* Create register pressure with many local variables */
    int r1 = get_value(), r2 = get_value(), r3 = get_value();
    int r4 = get_value(), r5 = get_value(), r6 = get_value();
    
    /* Loop with post-increment - should generate (plus (reg) (const_int 0)) */
    while (p < end) {
        sum += *p++;  /* Post-increment access */
        
        /* Use all register pressure variables to keep them live */
        r1 ^= sum; r2 += sum; r3 -= sum;
        r4 |= sum; r5 &= sum; r6 ^= ~sum;
    }
    
    /* Prevent optimization of register pressure vars */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6));
    
    return sum + r1 + r2 + r3 + r4 + r5 + r6;
}

/* Test 2: String copy with post-increment on both pointers */
void test2_str_copy(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    const char* end = src + n;
    
    /* Register pressure */
    int t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;
    
    /* Classic post-increment copy pattern */
    while (s < end) {
        *d++ = *s++;  /* Both pointers post-increment */
        
        /* Complex use of temporaries to increase register pressure */
        t1 += *d; t2 += *s;
        t3 = t1 ^ t2; t4 = t3 + t1; t5 = t4 - t2;
    }
    
    /* Force compiler to consider all variables */
    use_int(t1); use_int(t2); use_int(t3); use_int(t4); use_int(t5);
    use_ptr(d); use_ptr(s);
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array(const struct Data* arr, size_t n) {
    int total = 0;
    const struct Data* p = arr;
    
    /* High register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (size_t i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p->value;
        
        /* Complex addressing to create different patterns */
        acc1 += p->payload[0];
        acc2 += p->payload[1];
        acc3 ^= p->tag;
        
        /* Post-increment the pointer */
        const struct Data* old_p = p++;
        
        /* More operations to keep variables live */
        tmp1 = old_p->value * 2;
        tmp2 = old_p->payload[0] + old_p->payload[1];
        tmp3 = tmp1 ^ tmp2;
        tmp4 = acc1 + acc2 + acc3;
        
        /* Use asm to prevent optimization */
        asm volatile("" : : "r"(tmp1), "r"(tmp2), "r"(tmp3), "r"(tmp4));
    }
    
    return total + acc1 + acc2 + acc3;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int matrix[][10], size_t rows) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k = 0, l = 0, m = 0;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        int* end = row + 10;
        
        /* Inner loop with post-increment pointer */
        while (p < end) {
            sum += *p++;  /* Post-increment access */
            
            /* Index arithmetic that might decompose to base+0 */
            k += matrix[i][(p - row) % 10];
            l += i * (p - row);
            m ^= sum;
        }
        
        /* Additional post-increment in loop update */
        int* q = matrix[i];
        for (j = 0; j < 10; j++) {
            int val = *(q + j);  /* This may become (plus (reg) (const_int 0)) */
            sum -= val;
            k += val;
        }
    }
    
    return sum + k + l + m;
}

/* Test 5: Mixed pointer types and arithmetic */
int test5_mixed_pointers(void* base, size_t n) {
    int sum = 0;
    char* cptr = (char*)base;
    int* iptr = (int*)base;
    
    /* Alternate between char and int pointers */
    for (size_t i = 0; i < n; i++) {
        /* Char access with post-increment */
        sum += (int)(*cptr++);
        
        /* Int access with stride */
        if (i % 4 == 0) {
            sum += *iptr;
            iptr += 1;  /* This may become (plus (reg) (const_int 4)) */
        }
        
        /* Combined form that might decompose */
        char* tmp = cptr;
        cptr = tmp + 2;  /* Could become base+offset */
        sum += *tmp;
    }
    
    return sum;
}

/* Test 6: Do-while loop with post-decrement */
int test6_post_decrement(int* arr, size_t n) {
    int sum = 0;
    int* p = arr + n - 1;
    
    /* Register pressure variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    
    do {
        sum += *p--;  /* Post-decrement */
        
        /* Use all pressure variables */
        a ^= sum; b += sum; c -= sum;
        d |= sum; e &= sum;
        
        /* Force memory barrier */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
    } while (p >= arr);
    
    return sum + a + b + c + d + e;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    int int_array[100];
    char char_array[200];
    struct Data struct_array[50];
    int matrix[20][10];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        int_array[i] = (i * 37) & 0xFF;
    }
    
    for (int i = 0; i < 200; i++) {
        char_array[i] = (char)((i * 13) & 0x7F);
    }
    
    for (int i = 0; i < 50; i++) {
        struct_array[i].value = i * 3;
        struct_array[i].tag = (char)i;
        struct_array[i].payload[0] = i * 2;
        struct_array[i].payload[1] = i * 5;
    }
    
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (i * 10 + j) * 7;
        }
    }
    
    int result = 0;
    
    /* Use command-line args to control which tests run */
    if (argc > 1) {
        char test_id = argv[1][0];
        
        switch (test_id) {
            case '1':
                result = test1_sum_int_array(int_array, 100);
                break;
            case '2':
                test2_str_copy(char_array, char_array + 50, 50);
                result = char_array[0];
                break;
            case '3':
                result = test3_struct_array(struct_array, 50);
                break;
            case '4':
                result = test4_nested_loops(matrix, 20);
                break;
            case '5':
                result = test5_mixed_pointers(int_array, 100);
                break;
            case '6':
                result = test6_post_decrement(int_array, 100);
                break;
            default:
                /* Run all tests */
                result = test1_sum_int_array(int_array, 100);
                test2_str_copy(char_array, char_array + 50, 50);
                result += test3_struct_array(struct_array, 50);
                result += test4_nested_loops(matrix, 20);
                result += test5_mixed_pointers(int_array, 100);
                result += test6_post_decrement(int_array, 100);
                break;
        }
    } else {
        /* Default: run all tests */
        result = test1_sum_int_array(int_array, 100);
        test2_str_copy(char_array, char_array + 50, 50);
        result += test3_struct_array(struct_array, 50);
        result += test4_nested_loops(matrix, 20);
        result += test5_mixed_pointers(int_array, 100);
        result += test6_post_decrement(int_array, 100);
    }
    
    /* Update global volatile to prevent optimization */
    global_checksum = result;
    
    return result != 0 ? 0 : 1;
}

/* Dummy implementations of opaque functions */
void __attribute__((noinline)) use_int(int x) {
    global_checksum ^= x;
}

void __attribute__((noinline)) use_ptr(void* p) {
    global_checksum += (int)((size_t)p & 0xFF);
}

void __attribute__((noinline)) use_char(char c) {
    global_checksum += (int)c;
}

int __attribute__((noinline)) get_value(void) {
    static int counter = 1;
    return counter++ & 0xF;
}
