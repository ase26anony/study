/* Test program to trigger specific RTL patterns in mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile to force memory operations */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int part1 : 8;
    unsigned int part2 : 8;
    unsigned int part3 : 8;
    unsigned int part4 : 8;
} __attribute__((packed));

volatile struct bitfield_struct bf;

NOINLINE void test_zero_extract(void) {
    /* Writing to bit-fields often generates ZERO_EXTRACT */
    bf.part1 = (global_counter & 0xFF);
    bf.part2 = ((global_counter >> 8) & 0xFF);
    
    /* Alternative using __builtin_bitfield */
    unsigned int val = global_counter;
    unsigned int mask = 0xFF00;
    unsigned int field = (val & mask) >> 8;
    
    /* Force compiler to consider bit-field operations */
    asm volatile("" : "+r"(val), "+r"(field));
    
    /* Complex bit-field assignment that might generate ZERO_EXTRACT */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 24;
    } packed;
    
    packed.a = global_counter & 0x7;
    packed.b = (global_counter >> 3) & 0x1F;
    packed.c = global_counter >> 8;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOINLINE void test_strict_low_part(void) {
    volatile char char_var;
    volatile short short_var;
    volatile int int_var = global_counter;
    
    /* These assignments to smaller types may generate STRICT_LOW_PART */
    char_var = (char)int_var;          /* Low byte assignment */
    short_var = (short)int_var;        /* Low word assignment */
    
    /* Inline assembly with % modifier for low part (x86 specific) */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (result)
        : "r" (global_counter)
        : "%eax"
    );
    
    /* Another approach using union for type punning */
    union {
        int full;
        struct {
            char low_byte;
            char byte1;
            char byte2;
            char high_byte;
        } parts;
    } u;
    
    u.full = global_counter;
    u.parts.low_byte = 0x42;  /* This might generate STRICT_LOW_PART */
}

/* ========== SUBREG Pattern ========== */
NOINLINE void test_subreg(void) {
    /* Operations on sub-registers often generate SUBREG */
    int32_t full = global_counter;
    int16_t half1, half2;
    
    /* Type punning through union to force sub-register access */
    union {
        uint32_t dword;
        uint16_t words[2];
        uint8_t bytes[4];
    } converter;
    
    converter.dword = global_counter;
    half1 = converter.words[0];  /* This may involve SUBREG */
    half2 = converter.words[1];
    
    /* Vector operations can generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[0];  /* Element extraction may use SUBREG */
    
    /* Packed structure access */
    struct __attribute__((packed)) packed_data {
        char a;
        int b;
        char c;
    } pd;
    
    pd.b = global_counter;
    int extracted = pd.b;  /* Unaligned access may involve SUBREG */
}

/* ========== MEM_P with Complex Addressing ========== */
NOINLINE void test_complex_mem(void) {
    /* Create complex addressing modes */
    volatile int array[100][100];
    volatile int *ptr_array[100];
    
    /* Initialize pointer array */
    for (int i = 0; i < 100; i++) {
        ptr_array[i] = &array[i][0];
    }
    
    /* Complex memory access with multiple indices */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* This generates complex addressing: *(ptr_array[i] + j * 10 + k) */
            sum += array[i][j * 10 + global_counter % 10];
        }
    }
    
    /* Structure with nested arrays */
    struct nested {
        int data[10][10];
        struct nested *next;
    } node1, node2;
    
    node1.next = &node2;
    node2.next = &node1;
    
    /* Complex chain of memory accesses */
    int val = node1.next->next->data[global_counter % 10][global_counter % 5];
    
    /* Pointer arithmetic with multiple offsets */
    int *base_ptr = &array[0][0];
    int offset1 = global_counter % 50;
    int offset2 = (global_counter * 3) % 50;
    
    /* Very complex addressing expression */
    volatile int complex_access = *(base_ptr + offset1 * 10 + offset2);
    
    /* Force compiler to keep all these computations */
    asm volatile("" : "+r"(sum), "+r"(val), "+r"(complex_access));
}

/* ========== Combined Test Function ========== */
NOINLINE void test_combined(void) {
    /* Test all patterns in one function to maximize coverage */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int field1 : 4;
        unsigned int field2 : 12;
        unsigned int field3 : 16;
    } bits;
    
    bits.field1 = global_counter & 0xF;
    bits.field2 = (global_counter >> 4) & 0xFFF;
    
    /* STRICT_LOW_PART via small type assignment */
    volatile short low_part;
    low_part = (short)global_counter;
    
    /* SUBREG via type punning */
    union {
        long long big;
        int halves[2];
    } u;
    u.big = global_counter;
    int half = u.halves[0];
    
    /* Complex MEM access */
    volatile int multi_array[20][20];
    int idx1 = global_counter % 10;
    int idx2 = (global_counter * 7) % 10;
    int complex_val = multi_array[idx1 + 5][idx2 + 5];
    
    /* Use all results to prevent elimination */
    asm volatile("" : "+r"(half), "+r"(complex_val));
}

/* ========== Main Function ========== */
int main(void) {
    /* Initialize global counter to prevent constant propagation */
    global_counter = 12345;
    
    /* Execute all test functions */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined();
    
    /* Additional iterations to ensure coverage */
    for (int i = 0; i < 10; i++) {
        global_counter += i;
        test_zero_extract();
        test_strict_low_part();
    }
    
    return 0;
}
