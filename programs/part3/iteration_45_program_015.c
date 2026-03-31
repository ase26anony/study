/* test-auto-inc-dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358: find_inc(true) with mem_loc = address_of_x, reg1_val = 0
 */

#include <stddef.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));
extern void barrier(void) __attribute__((noinline));

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
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    while (p < end) {
        /* Post-increment access - should create (plus (reg) (const_int 0)) */
        sum += *p++;
        
        /* Use many variables to increase register pressure */
        temp1 = sum * 2;
        temp2 = temp1 + 1;
        temp3 = temp2 * 3;
        temp4 = temp3 - sum;
        
        acc1 += temp1;
        acc2 += temp2;
        acc3 += temp3;
        acc4 += temp4;
        
        /* Prevent optimization of pointer */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Combine all accumulators to ensure they're used */
    sum += acc1 + acc2 + acc3 + acc4;
    
    /* Opaque use to prevent elimination */
    use_int(sum);
    return sum;
}

/* Test 2: String copy with post-increment pointers */
void test2_str_copy(char* dst, const char* src, size_t n) {
    char* d = dst;
    const char* s = src;
    size_t i = 0;
    
    /* Register pressure variables */
    int checksum = 0;
    char c1, c2, c3, c4;
    
    /* Mixed loop structure - do-while with post-increment */
    if (n > 0) {
        do {
            /* Classic post-increment copy pattern */
            *d++ = *s++;
            
            /* Additional operations on the characters */
            c1 = *(s - 1);
            c2 = c1 + 1;
            c3 = c2 * 2;
            c4 = c3 - c1;
            
            checksum += c1 + c2 + c3 + c4;
            i++;
            
            /* Make pointers appear used */
            asm volatile("" : : "r"(d), "r"(s));
        } while (i < n);
    }
    
    /* Prevent dead code elimination */
    use_char(checksum & 0xFF);
    use_ptr(dst);
}

/* Test 3: Struct array traversal with post-increment */
int test3_struct_array(const struct Data* arr, size_t n) {
    int total = 0;
    const struct Data* p = arr;
    
    /* Many local variables for register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8;
    v1 = v2 = v3 = v4 = v5 = v6 = v7 = v8 = 0;
    
    for (size_t i = 0; i < n; i++) {
        /* Access struct member via post-increment pointer */
        total += p->value;
        
        /* Complex expression with the pointer */
        v1 += p->payload[0];
        v2 += p->payload[1];
        v3 += p->tag;
        v4 += (int)p->tag * 2;
        
        /* Post-increment the pointer */
        p++;
        
        /* More calculations to use variables */
        v5 = v1 * v2;
        v6 = v3 + v4;
        v7 = v5 - v6;
        v8 += v7;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    total += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    use_int(total);
    return total;
}

/* Test 4: Nested loops with array indexing and post-increment */
int test4_nested_loops(int matrix[][10], size_t rows) {
    int sum = 0;
    
    /* Multiple index variables for register pressure */
    int i, j, k, l;
    int tmp1, tmp2, tmp3, tmp4;
    tmp1 = tmp2 = tmp3 = tmp4 = 0;
    
    for (i = 0; i < rows; i++) {
        int* row = matrix[i];
        int* p = row;
        int* end = row + 10;
        
        /* Inner loop with pointer post-increment */
        j = 0;
        while (p < end) {
            /* This should generate base+0 addressing */
            sum += *p++;
            
            /* Additional index-based access */
            k = j++;
            l = k * 2;
            
            tmp1 += matrix[i][k % 10];
            tmp2 += l;
            tmp3 += k;
            tmp4 += i;
            
            /* Complex expression to use all variables */
            asm volatile("" : : "r"(p), "r"(row), "r"(k), "r"(l));
        }
    }
    
    sum += tmp1 + tmp2 + tmp3 + tmp4;
    use_int(sum);
    return sum;
}

/* Test 5: Mixed pointer arithmetic forms */
int test5_mixed_forms(int* arr, size_t n, int stride) {
    int result = 0;
    int* p = arr;
    int* end = arr + n * stride;
    
    /* High register pressure */
    int a, b, c, d, e, f, g, h;
    a = b = c = d = e = f = g = h = 0;
    
    /* Different pointer update forms */
    while (p < end) {
        /* Form 1: Simple post-increment */
        a += *p++;
        
        /* Form 2: Pointer arithmetic in access */
        if (p < end) {
            b += *(p + 0);  /* Should become same as *p */
            p += 1;
        }
        
        /* Form 3: Compound assignment */
        if (p < end) {
            c += *p;
            p += 1;
        }
        
        /* Use all accumulator variables */
        d = a * b;
        e = c + d;
        f = e - a;
        g = f * 2;
        h += g;
        
        /* Force register usage */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), 
                          "r"(e), "r"(f), "r"(g), "r"(h));
    }
    
    result = a + b + c + d + e + f + g + h;
    use_int(result);
    return result;
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Initialize test data */
    int int_array[100];
    char char_array[100];
    struct Data struct_array[50];
    int matrix[5][10];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 100; i++) {
        int_array[i] = (i * 3) % 97;
        char_array[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 50; i++) {
        struct_array[i].value = i * 2;
        struct_array[i].tag = 'a' + (i % 26);
        struct_array[i].payload[0] = i;
        struct_array[i].payload[1] = i * 3;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Use command-line arguments to control which tests run */
    int test_mask = 0;
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    } else {
        test_mask = 0x1F;  /* Run all tests by default */
    }
    
    int total = 0;
    
    if (test_mask & 0x01) {
        total += test1_sum_int_array(int_array, 100);
    }
    
    if (test_mask & 0x02) {
        test2_str_copy(char_array, char_array + 50, 50);
        total += char_array[25];  /* Use result */
    }
    
    if (test_mask & 0x04) {
        total += test3_struct_array(struct_array, 50);
    }
    
    if (test_mask & 0x08) {
        total += test4_nested_loops(matrix, 5);
    }
    
    if (test_mask & 0x10) {
        total += test5_mixed_forms(int_array, 25, 2);
    }
    
    /* Update global volatile to prevent optimization */
    global_checksum = total;
    
    return total != 0 ? 0 : 1;
}

/* Dummy implementations of opaque functions */
void use_int(int x) {
    global_checksum += x;
}

void use_ptr(void* p) {
    global_checksum += (int)((size_t)p & 0xFFFF);
}

void use_char(char c) {
    global_checksum += c;
}

void barrier(void) {
    asm volatile("" ::: "memory");
}
