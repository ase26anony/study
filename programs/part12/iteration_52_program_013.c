/* test_resource.c - Test program to trigger specific RTL patterns in mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to trigger ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int part : 8;
    unsigned int other : 16;
} __attribute__((packed));

volatile struct bitfield_struct bf;

NOOPT void test_zero_extract(void) {
    /* Writing to a bit-field should generate ZERO_EXTRACT */
    bf.part = 0xAB;  /* This should generate ZERO_EXTRACT for the bit-field store */
    
    /* Another approach using __builtin_bitfield */
    unsigned int val = 0x12345678;
    /* Extract bits 8-15 and store them in a variable */
    unsigned int extracted = __builtin_bitfield_extract(val, 8, 8);
    /* Store back to a different position - might generate ZERO_EXTRACT */
    val = __builtin_bitfield_insert(val, extracted, 16, 8);
    
    /* Force use of results */
    global_counter += bf.part + extracted + val;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOOPT void test_strict_low_part(void) {
    volatile char char_var;
    volatile short short_var;
    volatile int int_var = 0x12345678;
    
    /* These assignments to smaller types might generate STRICT_LOW_PART */
    char_var = (char)int_var;  /* Low byte assignment */
    short_var = (short)int_var; /* Low word assignment */
    
    /* Inline assembly with low-part modifier for x86 */
    int input = 0xABCD;
    int output;
    
    /* Using %b0 for low byte, %h0 for high byte, %w0 for word on x86 */
    asm volatile (
        "movl %1, %0\n\t"
        "movb %b0, %b0\n\t"  /* Operate on low byte */
        : "=r"(output)
        : "r"(input)
        : "cc"
    );
    
    global_counter += char_var + short_var + output;
}

/* ========== SUBREG Pattern ========== */
NOOPT void test_subreg(void) {
    /* Using unions for type-punning */
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } data;
    
    data.full = 0xDEADBEEF;
    
    /* Operations on sub-parts should generate SUBREG */
    data.halves[0] += 1;      /* Might use SUBREG for 16-bit access */
    data.bytes[2] = 0xCC;     /* Might use SUBREG for 8-bit access */
    
    /* Vector types can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];      /* Element extraction might use SUBREG */
    
    /* Packed structure */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
    } ps;
    
    ps.b = 0x12345678;        /* Misaligned access might involve SUBREG */
    
    global_counter += data.full + element + ps.b;
}

/* ========== MEM_P with Complex Addressing ========== */
#define ARRAY_SIZE 100

struct nested {
    int values[10];
    struct nested *next;
};

NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    int multi_array[10][10][10];
    volatile int sum = 0;
    
    /* Complex addressing expression */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                /* This should generate complex address expressions */
                multi_array[i][j][k] = i * 100 + j * 10 + k;
                sum += multi_array[i][j][k];
            }
        }
    }
    
    /* Structure with pointer chains */
    struct nested chain[5];
    for (int i = 0; i < 4; i++) {
        chain[i].next = &chain[i + 1];
        for (int j = 0; j < 10; j++) {
            chain[i].values[j] = i * 10 + j;
        }
    }
    chain[4].next = NULL;
    
    /* Complex memory access through pointer chain */
    struct nested *current = &chain[0];
    while (current && current->next) {
        /* Access with offset calculation */
        sum += current->values[5] + current->next->values[3];
        current = current->next;
    }
    
    /* Inline assembly with memory operand */
    int mem_value;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(mem_value)
        : "m"(*(int*)((char*)&chain[0] + offsetof(struct nested, values[2])))
        : "memory"
    );
    
    global_counter += sum + mem_value;
}

/* ========== Combined Test ========== */
NOOPT void test_combined(void) {
    /* Try to trigger multiple patterns in one function */
    
    /* ZERO_EXTRACT via bit-field in struct */
    struct {
        unsigned int field1 : 4;
        unsigned int field2 : 12;
        unsigned int field3 : 16;
    } bits;
    
    bits.field2 = 0xABC;  /* Potential ZERO_EXTRACT */
    
    /* STRICT_LOW_PART via byte store */
    volatile unsigned char *byte_ptr = (volatile unsigned char*)&bits;
    byte_ptr[2] = 0xEF;  /* Might use STRICT_LOW_PART */
    
    /* SUBREG via type punning */
    union {
        uint64_t dword;
        uint32_t words[2];
    } u;
    u.dword = 0x123456789ABCDEF0ULL;
    u.words[1] ^= 0xFFFF;  /* Might use SUBREG */
    
    /* Complex MEM access */
    int array[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            array[i][j] = i * j;
            global_counter += array[i][j];  /* Complex address calculation */
        }
    }
    
    global_counter += bits.field2 + u.words[0];
}

/* ========== Main Function ========== */
int main(void) {
    /* Initialize to prevent constant propagation */
    bf.full = 0;
    bf.part = 0;
    bf.other = 0;
    
    /* Run all tests multiple times to ensure execution */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    /* Dummy return based on global_counter to prevent dead code elimination */
    return global_counter == 0 ? 0 : 1;
}
