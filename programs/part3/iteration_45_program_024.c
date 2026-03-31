/* test-auto-inc-dec.c
 * Program designed to trigger specific uncovered lines in GCC's auto-inc-dec pass
 * Lines 1352-1358 in auto-inc-dec.cc: find_inc(true) with reg1_val = 0
 */

#include <stddef.h>
#include <string.h>

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Opaque external functions to create aliasing concerns */
extern void use_int_ptr(int *p) __attribute__((noinline, noipa));
extern void use_char_ptr(char *p) __attribute__((noinline, noipa));
extern void use_void_ptr(void *p) __attribute__((noinline, noipa));

/* Struct to create complex memory access patterns */
struct Data {
    int value;
    char tag;
    int payload[2];
};

/* Prevent inlining to preserve patterns */
__attribute__((noinline))
int test_post_increment_sum(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    /* Create register pressure with many live variables */
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    /* Loop with post-increment pointer access */
    while (p < end) {
        /* Pattern: *p++ creates mem_insn.mem_loc = address_of_x with reg0 = p, reg1_val = 0 */
        sum += *p++;
        
        /* Use many variables to increase register pressure */
        temp1 += sum & 0xFF;
        temp2 += sum >> 8;
        temp3 += temp1 * 2;
        temp4 += temp2 / 3;
        temp5 ^= temp3;
        temp6 |= temp4;
        temp7 = temp5 + temp6;
        temp8 = temp7 - temp1;
        
        /* Make pointer appear used to prevent optimization */
        asm volatile("" : : "r"(p) : "memory");
    }
    
    /* Use all temps to prevent elimination */
    return sum + temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
}

__attribute__((noinline))
void test_char_ptr_copy(char *dst, const char *src, size_t len) {
    /* Create register pressure */
    char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    int count = 0;
    
    /* Classic *dst++ = *src++ pattern */
    for (size_t i = 0; i < len; i++) {
        *dst++ = *src++;
        count++;
        
        /* Use variables to maintain register pressure */
        c1 = *dst;
        c2 = *src;
        c3 = c1 ^ c2;
        c4 = c3 + count;
        
        /* Opaque function call creates aliasing */
        use_char_ptr(dst);
        use_char_ptr((char*)src);
    }
    
    /* Prevent elimination */
    global_checksum += c1 + c2 + c3 + c4 + count;
}

__attribute__((noinline))
int test_struct_array_traversal(struct Data *arr, int n) {
    int total = 0;
    struct Data *p = arr;
    
    /* Create high register pressure */
    int reg_pressure[8] = {0};
    
    for (int i = 0; i < n; i++) {
        /* Access struct member via post-increment pointer */
        total += p->value;
        
        /* Complex expression with pointer arithmetic */
        int *payload_ptr = p->payload;
        total += *payload_ptr++;
        total += *payload_ptr++;
        
        /* Post-increment the struct pointer */
        p++;
        
        /* Maintain register pressure */
        for (int j = 0; j < 8; j++) {
            reg_pressure[j] += total + j;
        }
        
        /* Opaque use to prevent optimization */
        use_void_ptr(p);
    }
    
    /* Use all pressure variables */
    int pressure_sum = 0;
    for (int j = 0; j < 8; j++) {
        pressure_sum += reg_pressure[j];
    }
    
    return total + pressure_sum;
}

__attribute__((noinline))
int test_nested_loops_with_index(int *matrix, int rows, int cols) {
    int sum = 0;
    
    /* Create many local variables for register pressure */
    int vars[16];
    for (int i = 0; i < 16; i++) vars[i] = i;
    
    for (int row = 0; row < rows; row++) {
        int *row_ptr = matrix + row * cols;
        int col = 0;
        
        /* Inner loop with post-increment on index */
        while (col < cols) {
            /* Array indexing with post-increment: row_ptr[col++] */
            sum += row_ptr[col++];
            
            /* Use many variables to maintain pressure */
            for (int i = 0; i < 16; i++) {
                vars[i] += sum + col + row;
            }
            
            /* Prevent optimization */
            asm volatile("" : : "r"(row_ptr), "r"(col) : "memory");
        }
        
        /* Opaque function call */
        use_int_ptr(row_ptr);
    }
    
    /* Use all variables */
    int var_sum = 0;
    for (int i = 0; i < 16; i++) {
        var_sum += vars[i];
    }
    
    return sum + var_sum;
}

__attribute__((noinline))
int test_mixed_pointer_arithmetic(int *arr, int n, int stride) {
    int result = 0;
    int *p = arr;
    int *end = arr + n * stride;
    
    /* Combined forms that may decompose to base+offset */
    while (p < end) {
        /* *(p += stride) - may create interesting patterns */
        result += *(p += stride);
        
        /* Create side effects to prevent reordering */
        int *q = p - stride;
        result -= *q;
        
        /* Complex expression to maintain pattern */
        int *r = p + 1;
        asm volatile("" : : "r"(r), "r"(q) : "memory");
        
        /* More register pressure */
        int temp = result;
        for (int i = 0; i < 8; i++) {
            temp = (temp << 1) | (temp >> 31);
            result ^= temp;
        }
    }
    
    return result;
}

/* External function definitions (simulated) */
void use_int_ptr(int *p) {
    global_checksum += (int)(intptr_t)p;
}

void use_char_ptr(char *p) {
    global_checksum += *p;
}

void use_void_ptr(void *p) {
    global_checksum += (int)(intptr_t)p;
}

int main(int argc, char *argv[]) {
    /* Initialize test data */
    int int_array[100];
    char char_array[100];
    struct Data struct_array[50];
    int matrix[10][10];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i;
        char_array[i] = (char)(i % 256);
    }
    
    for (int i = 0; i < 50; i++) {
        struct_array[i].value = i * 2;
        struct_array[i].tag = (char)i;
        struct_array[i].payload[0] = i;
        struct_array[i].payload[1] = i + 1;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Use command-line arguments to control execution path */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 5;
    }
    
    int result = 0;
    
    /* Execute different test patterns based on input */
    switch (test_case) {
        case 0:
            result = test_post_increment_sum(int_array, 100);
            break;
        case 1:
            test_char_ptr_copy(char_array, char_array + 50, 50);
            result = global_checksum;
            break;
        case 2:
            result = test_struct_array_traversal(struct_array, 50);
            break;
        case 3:
            result = test_nested_loops_with_index(&matrix[0][0], 10, 10);
            break;
        case 4:
            result = test_mixed_pointer_arithmetic(int_array, 20, 3);
            break;
        default:
            result = -1;
    }
    
    /* Ensure result is used */
    global_checksum += result;
    
    return global_checksum != 0 ? 0 : 1;
}
