/* test-auto-inc-dec.c
 * Program designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358: find_inc(true) with mem_insn.reg1_val = 0
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
};

/* Prevent inlining to preserve patterns */
__attribute__((noinline))
int test_post_increment_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* Access memory with post-increment */
        sum += *p++;
        
        /* Use many variables to create register pressure */
        r1 += sum;
        r2 = r1 * 2;
        r3 = r2 - r1;
        r4 = r3 ^ r2;
        r5 = r4 | r3;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(sum), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5));
    }
    
    /* Use all variables to prevent elimination */
    sum += r1 + r2 + r3 + r4 + r5;
    return sum;
}

__attribute__((noinline))
void test_char_array_copy(char* dst, const char* src, int n) {
    int i = 0;
    
    /* Create register pressure */
    char c1, c2, c3, c4;
    int t1 = 0, t2 = 0, t3 = 0;
    
    /* Copy with post-increment */
    while (i < n) {
        *dst++ = *src++;
        i++;
        
        /* Additional operations to create register pressure */
        c1 = *src;
        c2 = c1 + 1;
        c3 = c2 * 2;
        c4 = c3 - c1;
        
        t1 += c1;
        t2 += c2;
        t3 += c3;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(dst), "r"(src), "r"(c1), "r"(c2), "r"(c3), "r"(c4));
    }
    
    /* Use variables */
    dst[-1] = c4;
    use_int(t1 + t2 + t3);
}

__attribute__((noinline))
int test_struct_array_traverse(struct Data* data, int n) {
    int total = 0;
    struct Data* p = data;
    
    /* Create significant register pressure */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access struct member with post-increment */
        total += p++->value;
        
        /* Complex calculations to use many registers */
        a = total * 2;
        b = a + i;
        c = b ^ a;
        d = c - total;
        e = d | c;
        f = e & b;
        g = f * 3;
        h = g / 2;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(p), "r"(total), "r"(a), "r"(b), "r"(c), 
                     "r"(d), "r"(e), "r"(f), "r"(g), "r"(h));
    }
    
    return total + a + b + c + d + e + f + g + h;
}

__attribute__((noinline))
int test_nested_loops_with_index(int* matrix, int rows, int cols) {
    int sum = 0;
    
    /* Create register pressure */
    int r[10];
    for (int i = 0; i < 10; i++) r[i] = i;
    
    for (int i = 0; i < rows; i++) {
        int* row_ptr = matrix + i * cols;
        int j = 0;
        
        /* Inner loop with post-increment on pointer */
        while (j < cols) {
            /* This should create (plus (reg) (const_int 0)) pattern */
            sum += row_ptr[j++];
            
            /* Use register pressure variables */
            for (int k = 0; k < 10; k++) {
                r[k] += sum + j;
            }
            
            /* Prevent optimization */
            asm volatile("" : : "r"(row_ptr), "r"(j), "r"(sum));
        }
        
        /* Use register variables */
        for (int k = 0; k < 10; k++) {
            sum += r[k];
        }
    }
    
    return sum;
}

__attribute__((noinline))
int test_mixed_pointer_arithmetic(int* arr, int n, int stride) {
    int result = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Multiple pointer types and arithmetic */
    char* cp = (char*)arr;
    short* sp = (short*)arr;
    
    /* Create register pressure */
    int accum[8] = {0};
    
    /* Loop with combined pointer arithmetic */
    while (p < end) {
        /* Different forms of pointer access */
        result += *p;           /* Simple dereference */
        p += stride;            /* Pointer increment */
        
        /* Access via different pointer types */
        accum[0] += *cp++;
        accum[1] += *sp++;
        
        /* More register pressure */
        for (int i = 2; i < 8; i++) {
            accum[i] += result + i;
        }
        
        /* Force pointer to be in register */
        use_ptr(p);
        use_ptr(cp);
        use_ptr(sp);
        
        /* Memory barrier */
        barrier();
    }
    
    /* Combine all accumulators */
    for (int i = 0; i < 8; i++) {
        result += accum[i];
    }
    
    return result;
}

/* Opaque function implementations */
void use_int(int x) {
    global_sum += x;
}

void use_ptr(void* p) {
    /* Access memory to prevent elimination */
    if (p) global_sum += 1;
}

void use_char(char c) {
    global_sum += c;
}

void barrier(void) {
    asm volatile("" ::: "memory");
}

int main(int argc, char** argv) {
    /* Initialize test data */
    const int N = 1024;
    int* int_array = malloc(N * sizeof(int));
    char* char_array = malloc(N * sizeof(char));
    struct Data* struct_array = malloc(N * sizeof(struct Data));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < N; i++) {
        int_array[i] = (i * 3) % 97;
        char_array[i] = 'A' + (i % 26);
        struct_array[i].value = i * 2;
        struct_array[i].tag = 'a' + (i % 26);
        struct_array[i].id = i;
    }
    
    int result = 0;
    
    /* Use command line arguments to control flow */
    int test_case = argc > 1 ? atoi(argv[1]) % 5 : 0;
    
    switch (test_case) {
        case 0:
            result = test_post_increment_sum(int_array, N);
            break;
        case 1:
            test_char_array_copy(char_array, char_array + N/2, N/2);
            result = char_array[N/2 - 1];
            break;
        case 2:
            result = test_struct_array_traverse(struct_array, N);
            break;
        case 3:
            result = test_nested_loops_with_index(int_array, 32, 32);
            break;
        case 4:
            result = test_mixed_pointer_arithmetic(int_array, N, 2);
            break;
    }
    
    /* Ensure results are used */
    global_sum += result;
    
    /* Print to prevent elimination */
    printf("Result: %d, Global: %d\n", result, global_sum);
    
    free(int_array);
    free(char_array);
    free(struct_array);
    
    return 0;
}
