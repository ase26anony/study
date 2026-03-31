/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Bit-field operations often generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 4;
    unsigned int padding : 16;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Writing to bit-fields should generate ZERO_EXTRACT */
    bf.field1 = 7;          /* 4-bit field */
    bf.field2 = 255;        /* 8-bit field */
    bf.field3 = 15;         /* 4-bit field */
    
    /* Complex bit-field expression */
    bf.field2 = (bf.field1 << 2) | (bf.field3 >> 1);
    
    global_counter += bf.field1 + bf.field2 + bf.field3;
}

/* Alternative using __builtin_bitfield */
NOOPT void test_zero_extract_builtin(void) {
    volatile uint32_t value = 0x12345678;
    
    /* Extract and modify bit-field */
    uint32_t field = __builtin_bitfield_extract(value, 8, 8);  /* Extract bits 8-15 */
    __builtin_bitfield_insert(value, field + 1, 8, 8);         /* Modify and insert back */
    
    global_counter += value;
}

/* ===== STRICT_LOW_PART Pattern ===== */
/* Partial register updates often generate STRICT_LOW_PART */
NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t char_var;
    volatile uint32_t int_var = 0x12345678;
    
    /* These assignments to smaller types may generate STRICT_LOW_PART */
    short_var = (uint16_t)(int_var + 1);      /* Low 16-bit assignment */
    char_var = (uint8_t)(int_var + 2);        /* Low 8-bit assignment */
    
    /* Inline assembly with %L0 modifier for x86 low-part constraint */
    uint32_t result;
    __asm__ volatile (
        "movl $0xDEADBEEF, %0\n\t"
        "movw $0x1234, %L0"   /* %L0 targets low 16 bits on x86 */
        : "=r" (result)
        :
        : "memory"
    );
    
    global_counter += short_var + char_var + result;
}

/* ===== SUBREG Pattern ===== */
/* Type punning and packed structures generate SUBREG */
struct packed_struct {
    uint16_t a;
    uint16_t b;
} __attribute__((packed));

NOOPT void test_subreg(void) {
    /* Type punning through union */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } data;
    
    data.full = 0x12345678;
    data.parts.low = 0xABCD;      /* May generate SUBREG */
    data.parts.high = 0xEF01;     /* May generate SUBREG */
    
    /* Vector operations with sub-register access */
    typedef uint16_t v4hi __attribute__((vector_size(8)));
    v4hi vec = {1, 2, 3, 4};
    uint16_t element = vec[2];    /* May generate SUBREG for extraction */
    
    /* Packed structure access */
    struct packed_struct ps;
    ps.a = 100;
    ps.b = 200;
    uint32_t combined = (ps.b << 16) | ps.a;  /* May involve SUBREG */
    
    global_counter += data.full + element + combined;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
/* Complex memory addressing expressions */
#define ARRAY_SIZE 100

struct nested {
    int data[10];
    struct nested *next;
};

NOOPT void test_complex_mem(void) {
    volatile int array[ARRAY_SIZE][ARRAY_SIZE];
    volatile struct nested complex_struct[50];
    volatile int *ptr_array[100];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing: multi-dimensional array with index arithmetic */
    int sum = 0;
    for (int i = 1; i < ARRAY_SIZE - 1; i++) {
        for (int j = 1; j < ARRAY_SIZE - 1; j++) {
            /* Complex address calculation */
            sum += array[i-1][j] + array[i][j-1] + array[i+1][j] + array[i][j+1];
        }
    }
    
    /* Structure pointer chain with complex addressing */
    for (int i = 0; i < 49; i++) {
        complex_struct[i].next = &complex_struct[i+1];
        for (int j = 0; j < 10; j++) {
            complex_struct[i].data[j] = i * 10 + j;
        }
    }
    
    /* Traverse linked list with complex memory access */
    struct nested *current = &complex_struct[0];
    while (current && current->next) {
        sum += current->data[5] + current->next->data[3];
        current = current->next;
    }
    
    /* Pointer array with complex indexing */
    for (int i = 0; i < 100; i++) {
        ptr_array[i] = &array[i % ARRAY_SIZE][(i * 7) % ARRAY_SIZE];
    }
    
    /* Very complex addressing expression */
    volatile int result = *(ptr_array[50] + 10 * (sum % 20) - 5);
    
    global_counter += sum + result;
}

/* ===== Combined Test Function ===== */
/* Function that combines multiple patterns */
NOOPT void test_combined(void) {
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int bits : 10;
    } bf;
    bf.bits = 511;
    
    /* STRICT_LOW_PART via partial assignment */
    volatile uint32_t val = 0x87654321;
    volatile uint16_t low_part = (uint16_t)val;
    
    /* SUBREG via type punning */
    union {
        uint64_t full;
        uint32_t halves[2];
    } u;
    u.full = 0x1122334455667788ULL;
    u.halves[0] = 0xAABBCCDD;
    
    /* Complex MEM access */
    volatile int matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    volatile int complex_access = matrix[5][5] + matrix[val % 10][low_part % 10];
    
    global_counter += bf.bits + low_part + u.halves[0] + complex_access;
}

/* ===== Main Function ===== */
int main(void) {
    /* Call all test functions multiple times to ensure they're not optimized away */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_zero_extract_builtin();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    /* Use the global counter to prevent dead code elimination */
    if (global_counter > 1000) {
        return 0;
    }
    return 1;
}
