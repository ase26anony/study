/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 0;
volatile int g_value = 42;
volatile int g_condition = 1;

/* Structure with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 3;   /* Will generate ZERO_EXTRACT */
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int padding : 16;
};

/* Vector types for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Global arrays for MEM operations */
int global_array[256];
struct bitfield_struct global_bf;

/* Function 1: Focus on ZERO_EXTRACT and MEM_P */
NOINLINE __attribute__((optimize("O2")))
void test_zero_extract_and_mem(int idx, int val) {
    struct bitfield_struct local_bf;
    int *ptr;
    
    /* ZERO_EXTRACT: Bit-field assignment */
    local_bf.field1 = val & 0x7;        /* Should generate ZERO_EXTRACT as SET_DEST */
    local_bf.field2 = (val >> 3) & 0x1F;
    
    /* MEM_P: Complex memory store with pointer arithmetic */
    ptr = &global_array[idx * 2 + 1];
    *ptr = val;                         /* MEM as SET_DEST with address expression */
    
    /* Another MEM with array indexing */
    global_array[(idx + 3) % 256] = local_bf.field1 + local_bf.field2;
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(local_bf.field1), "r"(ptr));
}

/* Function 2: Focus on STRICT_LOW_PART (partial register writes) */
NOINLINE __attribute__((optimize("O1")))
void test_strict_low_part(short src_short, char src_char) {
    int dest_int;
    long dest_long;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    dest_int = src_short;               /* short -> int, partial register write */
    dest_long = src_char;               /* char -> long, partial register write */
    
    /* Mixed-size operations to encourage partial register usage */
    dest_int = dest_int + (src_char * 2);
    
    /* Use volatile to prevent optimization */
    volatile int *volatile_ptr = &dest_int;
    *volatile_ptr = dest_int | 0xFF00;
    
    /* Force use of variables */
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
}

/* Function 3: Focus on SUBREG operations with vectors */
NOINLINE __attribute__((optimize("O3")))
void test_subreg_and_complex_mem(int idx) {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short;
    int temp_array[4];
    
    /* Vector operations that may involve SUBREG */
    vec_a = vec_a + vec_b;              /* Vector operation */
    
    /* Extract element from vector - may use SUBREG */
    int element = vec_a[idx % 4];       /* SUBREG as part of extraction */
    
    /* Type punning through union for SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    element = pun.i;                    /* May involve SUBREG */
    
    /* Complex MEM address calculation */
    int *mem_ptr = &global_array[(element * 17 + idx) % 256];
    *mem_ptr = element;                 /* MEM with complex address */
    
    /* Store vector element to memory */
    temp_array[0] = vec_a[0];
    temp_array[1] = vec_a[1];
    
    /* Prevent optimization */
    asm volatile("" : : "r"(vec_a), "r"(element), "r"(mem_ptr));
}

/* Function 4: Combined test with control flow */
NOINLINE __attribute__((optimize("O2")))
void test_combined(int idx, int val) {
    struct bitfield_struct bf;
    int array[10];
    short short_val;
    
    /* Initialize */
    bf.field1 = val & 0x7;
    short_val = val & 0xFFFF;
    
    /* Control flow to prevent single basic block */
    for (int i = 0; i < 5; i++) {
        if (i & 1) {
            /* ZERO_EXTRACT path */
            bf.field2 = (val + i) & 0x1F;
        } else {
            /* MEM_P path with SUBREG address */
            array[i] = bf.field1 + i;   /* MEM store */
        }
        
        /* STRICT_LOW_PART-like assignment */
        int temp = short_val;           /* short to int conversion */
        array[i] = temp;
    }
    
    /* Complex pointer expression for MEM */
    int *ptr = array + (idx % 10);
    *ptr = val * 2;
    
    /* Use all variables */
    asm volatile("" : : "r"(bf.field1), "r"(bf.field2), "r"(array[0]), "r"(ptr));
}

/* Helper to return pointer based on condition */
int* get_pointer(int idx, int cond) {
    if (cond) {
        return &global_array[idx % 256];
    } else {
        return &global_array[(idx * 3) % 256];
    }
}

/* Main function that calls all tests */
int main(int argc, char **argv) {
    int base_idx = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Initialize global data */
    memset(global_array, 0, sizeof(global_array));
    global_bf.field1 = 1;
    global_bf.field2 = 2;
    
    /* Test 1: ZERO_EXTRACT and MEM_P */
    for (int i = 0; i < 10; i++) {
        test_zero_extract_and_mem(base_idx + i, g_value + i);
    }
    
    /* Test 2: STRICT_LOW_PART */
    test_strict_low_part((short)g_value, (char)g_value);
    test_strict_low_part((short)(g_value * 2), (char)(g_value + 1));
    
    /* Test 3: SUBREG and complex MEM */
    for (int i = 0; i < 5; i++) {
        test_subreg_and_complex_mem(base_idx + i * 7);
    }
    
    /* Test 4: Combined */
    test_combined(base_idx, g_value);
    test_combined(base_idx + 50, g_value * 3);
    
    /* Additional MEM_P tests with helper function */
    int *ptr = get_pointer(base_idx, g_condition);
    *ptr = 0xABCD;
    
    ptr = get_pointer(base_idx + 1, !g_condition);
    *ptr = 0x1234;
    
    /* Use result to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += global_array[i];
    }
    
    return sum & 0xFF;
}
