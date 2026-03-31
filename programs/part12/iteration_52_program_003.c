/* test_resource.c - Comprehensive test for uncovered lines in resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int part1 : 8;
    volatile unsigned int part2 : 4;
    volatile unsigned int part3 : 12;
    volatile unsigned int part4 : 8;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to generate ZERO_EXTRACT */
    bf.part1 = 0xAB;           /* Should generate ZERO_EXTRACT for 8-bit field */
    bf.part2 = 0x7;            /* Should generate ZERO_EXTRACT for 4-bit field */
    bf.part3 = 0xABC;          /* Should generate ZERO_EXTRACT for 12-bit field */
    
    /* Complex bit-field operation */
    bf.part4 = (bf.part1 ^ bf.part2) | bf.part3;
    
    /* Using __builtin_bitfield to force ZERO_EXTRACT */
    unsigned int val = 0x12345678;
    unsigned int mask = 0xFF00;
    unsigned int field = __builtin_bitfield_extract(val, 8, 8);
    __builtin_bitfield_insert(val, field ^ 0xAA, 4, 8);
    
    global_counter += bf.part4 + field;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOOPT void test_strict_low_part(void) {
    volatile char byte_var;
    volatile short short_var;
    volatile int int_var = 0x12345678;
    
    /* These assignments often generate STRICT_LOW_PART on x86 */
    byte_var = (char)int_var;          /* Low byte assignment */
    short_var = (short)int_var;        /* Low word assignment */
    
    /* Inline assembly with %b0 (low byte) modifier for x86 */
    int result;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (result)
        : "r" (int_var)
        : "%eax"
    );
    
    /* Another approach: volatile struct with char members */
    struct {
        volatile char low_byte;
        volatile char high_byte;
        volatile short low_word;
    } parts;
    
    parts.low_byte = 0x42;
    parts.low_word = 0xABCD;
    
    global_counter += byte_var + short_var + result + parts.low_word;
}

/* ========== SUBREG Pattern ========== */
NOOPT void test_subreg(void) {
    /* Using packed structures to force SUBREG accesses */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
        short d;
    } ps;
    
    ps.b = 0xDEADBEEF;
    ps.d = 0x1234;
    
    /* Type punning through union */
    union type_pun {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } u;
    
    u.full = 0x87654321;
    u.parts.low = u.parts.high ^ 0x5555;
    
    /* Vector types that generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* This often uses SUBREG */
    
    /* Cast through different integer sizes */
    int32_t i32 = 0x12345678;
    int16_t i16 = (int16_t)i32;
    int32_t i32_again = i16 * 2;  /* Promotions use SUBREG */
    
    global_counter += ps.b + u.full + element + i32_again;
}

/* ========== MEM_P with Complex Addressing Pattern ========== */
#define ARRAY_SIZE 256

NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int multi_array[4][ARRAY_SIZE][ARRAY_SIZE];
    
    /* Complex addressing expression */
    int idx1 = global_counter % 64;
    int idx2 = (global_counter * 3) % 128;
    int idx3 = (global_counter + 7) % 256;
    
    /* This should generate complex address computation */
    multi_array[idx1][idx2][idx3] = 
        multi_array[idx1+1][idx2-1][idx3*2 % ARRAY_SIZE] +
        multi_array[idx1][idx2*3 % ARRAY_SIZE][idx3];
    
    /* Structure with pointer chains */
    struct node {
        volatile int value;
        struct node *next;
        struct node *prev;
    };
    
    struct node nodes[100];
    for (int i = 0; i < 99; i++) {
        nodes[i].next = &nodes[i+1];
        nodes[i].prev = (i > 0) ? &nodes[i-1] : 0;
        nodes[i].value = i * 7;
    }
    
    /* Complex pointer arithmetic */
    struct node *current = &nodes[50];
    for (int i = 0; i < 10; i++) {
        current->value = current->next->prev->value + 
                        current->prev->next->value;
        current = current->next->prev->next;
    }
    
    /* Inline assembly with memory clobber */
    int temp = 0;
    __asm__ volatile (
        "movl $100, %%ecx\n\t"
        "1:\n\t"
        "addl $1, %0\n\t"
        "loop 1b\n\t"
        : "+m" (temp)
        :
        : "%ecx", "memory"
    );
    
    global_counter += multi_array[0][0][0] + nodes[0].value + temp;
}

/* ========== Combined Test ========== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 11;
    } bf;
    bf.field1 = 0x1F;
    bf.field2 = bf.field1 << 2;
    
    /* STRICT_LOW_PART via char assignment */
    volatile int source = 0x89ABCDEF;
    volatile char dest;
    dest = (char)source;
    
    /* SUBREG via packed struct */
    struct __attribute__((packed)) {
        char a;
        int b;
    } ps;
    ps.b = source;
    int extracted = ps.b;
    
    /* Complex MEM access */
    volatile int array[100][100];
    int x = global_counter % 50;
    int y = (global_counter * 3) % 50;
    array[x][y] = array[y][x] + extracted + dest;
    
    global_counter += bf.field2 + array[0][0];
}

/* ========== Main Function ========== */
int main(void) {
    /* Initialize global counter */
    global_counter = 1;
    
    /* Execute all test functions multiple times */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Add some branching to prevent optimization */
        if (global_counter & 1) {
            global_counter *= 3;
        } else {
            global_counter /= 2;
        }
    }
    
    /* Return the final counter value to prevent dead code elimination */
    return global_counter & 0xFF;
}
