/* test_resource.c - Program to exercise specific RTL patterns in GCC's resource tracking */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 0;
volatile int g_value = 42;

/* Structure with bit-fields to generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field3 : 3;
    volatile unsigned int field5 : 5;
    volatile unsigned int field8 : 8;
};

/* Vector types to generate SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Function 1: Focus on ZERO_EXTRACT and MEM_P patterns */
NOINLINE __attribute__((optimize("O2")))
void test_zero_extract_and_mem(int *arr, int n) {
    struct bitfield_struct bfs = {0};
    
    /* This should generate ZERO_EXTRACT as SET_DEST */
    bfs.field3 = 5;      /* ZERO_EXTRACT destination */
    bfs.field5 = 31;     /* Another ZERO_EXTRACT */
    bfs.field8 = 255;    /* And another */
    
    /* Complex memory store - MEM_P destination */
    int idx = g_index % n;
    arr[idx] = g_value;  /* MEM destination with complex address */
    
    /* More complex addressing */
    arr[(idx * 3 + 1) % n] = bfs.field3 + bfs.field5;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bfs.field8));
}

/* Function 2: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE __attribute__((optimize("O1")))
void test_strict_low_part_and_subreg(short src_short, char src_char) {
    int dest_int = 0;
    long dest_long = 0;
    
    /* These may generate STRICT_LOW_PART on x86 */
    dest_int = src_short;   /* Possible STRICT_LOW_PART */
    dest_long = src_char;   /* Another possible STRICT_LOW_PART */
    
    /* Vector operations for SUBREG */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;
    
    /* Extract element - generates SUBREG */
    int element = vec_c[0];  /* SUBREG extraction */
    
    /* Mixed vector types - more SUBREG */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    short_vec[3] = src_short;  /* SUBREG store */
    
    /* Type punning through union - can generate SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    dest_int = pun.i;  /* Possible SUBREG */
    
    /* Use results to prevent optimization */
    asm volatile("" : : "r"(dest_int), "r"(dest_long), "r"(element));
}

/* Function 3: Mixed patterns with control flow */
NOINLINE __attribute__((optimize("O2")))
int test_mixed_patterns(int x, int y) {
    volatile int result = 0;
    
    /* Bit-field in loop - ZERO_EXTRACT */
    struct bitfield_struct bfs2;
    for (int i = 0; i < 3; i++) {
        bfs2.field3 = i & 0x7;  /* ZERO_EXTRACT in loop */
        result += bfs2.field3;
    }
    
    /* Memory store with condition - MEM_P */
    static int buffer[10];
    int idx = (x * y) % 10;
    if (idx > 0) {
        buffer[idx] = result;  /* Conditional MEM store */
    }
    
    /* Partial register write - STRICT_LOW_PART */
    short temp_short = x & 0xFFFF;
    int temp_int = temp_short;  /* Possible STRICT_LOW_PART */
    
    /* Vector extract - SUBREG */
    v4si vec = {x, y, x+y, x-y};
    temp_int += vec[1];  /* SUBREG extract */
    
    /* Complex addressing with pointer arithmetic */
    int *ptr = buffer + (y % 5);
    *ptr = temp_int;  /* MEM with pointer arithmetic */
    
    return result + temp_int;
}

/* Helper to create complex address expressions */
NOINLINE
int* get_complex_pointer(int *base, int offset) {
    return base + (offset * 2 + 1);
}

/* Function 4: Complex memory addressing patterns */
NOINLINE __attribute__((optimize("O3")))
void test_complex_mem(int *arr, int size) {
    /* Nested array access with computation */
    for (int i = 0; i < size; i++) {
        int *target = get_complex_pointer(arr, i);
        *target = i * g_value;  /* MEM with function-call address */
    }
    
    /* Struct with array member */
    struct {
        int data[5];
        int count;
    } s;
    
    s.count = size % 5;
    s.data[s.count] = 999;  /* MEM with struct+array indexing */
    
    /* Pointer to volatile for MEM_P */
    volatile int *volatile_ptr = arr;
    volatile_ptr[size/2] = 777;  /* Volatile MEM store */
}

/* Main function to drive all tests */
int main() {
    int array[20] = {0};
    
    /* Test 1: ZERO_EXTRACT and MEM_P */
    test_zero_extract_and_mem(array, 20);
    
    /* Test 2: STRICT_LOW_PART and SUBREG */
    test_strict_low_part_and_subreg(32767, 127);
    
    /* Test 3: Mixed patterns */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += test_mixed_patterns(i, i+1);
    }
    
    /* Test 4: Complex memory addressing */
    test_complex_mem(array, 20);
    
    /* Use results to prevent dead code elimination */
    asm volatile("" : : "r"(sum), "r"(array[0]));
    
    return 0;
}
