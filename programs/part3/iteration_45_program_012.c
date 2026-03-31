/* test_auto_inc_dec.c
 * Designed to trigger find_inc(true) path in GCC's auto-inc-dec optimization
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c test_auto_inc_dec.c -o test.o
 * Or: gcc -O3 -funroll-loops -fno-inline -c test_auto_inc_dec.c -o test.o
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
extern void use_int(int) __attribute__((noinline));
extern void use_ptr(void*) __attribute__((noinline));
extern void use_char(char) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    float weight;
};

/* Prevent inlining to preserve patterns */
__attribute__((noinline))
int test_post_increment_sum(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    int* end = arr + n;
    
    /* Create register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0;
    
    while (p < end) {
        /* Post-increment access - target for find_inc() */
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
        
        /* Use asm to prevent optimization */
        asm volatile("" : : "r"(p), "r"(sum), "r"(r1), "r"(r2), 
                     "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8));
    }
    
    /* Use pointer to create aliasing concerns */
    use_ptr(p);
    return sum + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

__attribute__((noinline))
void test_char_array_copy(char* dst, const char* src, size_t n) {
    /* Classic post-increment copy pattern */
    for (size_t i = 0; i < n; i++) {
        *dst++ = *src++;
    }
    
    /* Mixed pointer arithmetic */
    char* p = dst;
    for (int i = 0; i < 10; i++) {
        /* Multiple post-increment patterns */
        char c1 = *p++;
        char c2 = *p++;
        char c3 = *p++;
        
        use_char(c1);
        use_char(c2);
        use_char(c3);
        
        /* Create register pressure */
        int t1 = c1 + c2;
        int t2 = c2 * c3;
        int t3 = t1 ^ t2;
        int t4 = t3 << 1;
        
        asm volatile("" : : "r"(p), "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    }
}

__attribute__((noinline))
int test_struct_array_traversal(struct Data* data, int count) {
    int total = 0;
    struct Data* ptr = data;
    
    /* Nested loops with post-increment */
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 3; j++) {
            /* Access struct member with post-increment */
            total += ptr->value;
            
            /* Post-increment pointer after access */
            struct Data* current = ptr++;
            
            /* Complex expression to prevent optimization */
            float weight_sum = current->weight * 2.0f;
            int scaled = (int)(weight_sum * 100.0f);
            
            /* Register pressure */
            int tmp1 = total ^ scaled;
            int tmp2 = tmp1 << 2;
            int tmp3 = tmp2 | total;
            int tmp4 = tmp3 & scaled;
            
            asm volatile("" : : "r"(ptr), "r"(total), "r"(tmp1), 
                         "r"(tmp2), "r"(tmp3), "r"(tmp4));
        }
    }
    
    return total;
}

__attribute__((noinline))
int test_mixed_indexing(int* arr, int stride, int n) {
    int sum = 0;
    
    /* Combined pointer arithmetic that may decompose to base+0 */
    int* p = arr;
    for (int i = 0; i < n; i++) {
        /* This may generate (plus (reg) (const_int 0)) */
        sum += *(p += stride) - stride;
        
        /* Decrement version */
        sum -= *(p -= 1);
        
        /* More register pressure */
        int* q = p + 1;
        int* r = q - 2;
        
        int v1 = *q;
        int v2 = *r;
        int v3 = v1 * v2;
        int v4 = v3 + sum;
        int v5 = v4 ^ v1;
        int v6 = v5 | v2;
        
        asm volatile("" : : "r"(p), "r"(q), "r"(r), "r"(v1), 
                     "r"(v2), "r"(v3), "r"(v4), "r"(v5), "r"(v6));
    }
    
    return sum;
}

__attribute__((noinline))
int test_do_while_loop(short* values, int count) {
    int sum = 0;
    short* ptr = values;
    int i = 0;
    
    /* do-while with post-increment */
    do {
        sum += *ptr++;
        
        /* Multiple increments in loop update */
        i++;
        
        /* Heavy register usage */
        short* p2 = ptr + 1;
        short* p3 = ptr - 1;
        int v1 = *p2;
        int v2 = *p3;
        int v3 = v1 + v2;
        int v4 = v3 * sum;
        int v5 = v4 >> 2;
        int v6 = v5 << 1;
        int v7 = v6 ^ v4;
        int v8 = v7 | v3;
        
        asm volatile("" : : "r"(ptr), "r"(p2), "r"(p3), "r"(v1),
                     "r"(v2), "r"(v3), "r"(v4), "r"(v5), "r"(v6),
                     "r"(v7), "r"(v8));
    } while (i < count);
    
    return sum;
}

/* External function definitions */
void use_int(int x) {
    global_checksum ^= x;
}

void use_ptr(void* p) {
    global_checksum += (int)((size_t)p & 0xFF);
}

void use_char(char c) {
    global_checksum += c;
}

int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 100;
    int* int_array = malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = malloc(ARRAY_SIZE * sizeof(char));
    struct Data* struct_array = malloc(ARRAY_SIZE * sizeof(struct Data));
    short* short_array = malloc(ARRAY_SIZE * sizeof(short));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 97;
        char_array[i] = 'A' + (i % 26);
        struct_array[i].value = i * 2;
        struct_array[i].weight = i * 0.1f;
        short_array[i] = (short)(i * 5);
    }
    
    int result = 0;
    
    /* Use command line args to control flow */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            result = test_post_increment_sum(int_array, ARRAY_SIZE);
            break;
        case 1:
            test_char_array_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result = char_array[0];
            break;
        case 2:
            result = test_struct_array_traversal(struct_array, ARRAY_SIZE/4);
            break;
        case 3:
            result = test_mixed_indexing(int_array, 2, ARRAY_SIZE/2);
            break;
        case 4:
            result = test_do_while_loop(short_array, ARRAY_SIZE);
            break;
        default:
            /* Run all tests */
            result = test_post_increment_sum(int_array, ARRAY_SIZE);
            test_char_array_copy(char_array, char_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            result += test_struct_array_traversal(struct_array, ARRAY_SIZE/4);
            result += test_mixed_indexing(int_array, 2, ARRAY_SIZE/2);
            result += test_do_while_loop(short_array, ARRAY_SIZE);
            break;
    }
    
    /* Update global to prevent elimination */
    global_checksum += result;
    
    /* Print to ensure code runs */
    printf("Result: %d, Checksum: %d\n", result, global_checksum);
    
    free(int_array);
    free(char_array);
    free(struct_array);
    free(short_array);
    
    return 0;
}
