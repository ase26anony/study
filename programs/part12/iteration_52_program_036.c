/* test_resource.c - Coverage test for mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 20;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to generate ZERO_EXTRACT */
    bf.field1 = 5;           /* Should generate ZERO_EXTRACT for 4-bit field */
    bf.field2 = 0xAB;        /* Should generate ZERO_EXTRACT for 8-bit field */
    bf.field3 = 0x12345;     /* Should generate ZERO_EXTRACT for 20-bit field */
    
    /* Read back to prevent elimination */
    global_counter += bf.field1 + bf.field2 + bf.field3;
}

/* Alternative using __builtin_bitfield */
NOOPT void test_zero_extract_builtin(void) {
    volatile uint32_t value = 0x12345678;
    
    /* Write to bit-field using builtin (if supported) */
    uint32_t mask = 0xFF;  /* 8-bit mask */
    uint32_t pos = 8;      /* starting at bit 8 */
    
    /* This may generate ZERO_EXTRACT */
    __builtin_bitfield_write(value, mask, pos, 0xAA);
    
    global_counter += value;
}

/* ========== STRICT_LOW_PART Pattern ========== */
/* Force partial register updates */
NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t char_var;
    volatile uint32_t int_var = 0x12345678;
    
    /* These assignments may generate STRICT_LOW_PART */
    short_var = (uint16_t)int_var;      /* Low 16-bit assignment */
    char_var = (uint8_t)int_var;        /* Low 8-bit assignment */
    
    /* Inline assembly for explicit STRICT_LOW_PART on x86 */
    uint32_t reg_val;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "movb %b2, %b0"      /* %b0 modifier for low byte */
        : "=r" (reg_val)
        : "r" (0x12345678), "r" (0xAA)
        : "cc"
    );
    
    global_counter += short_var + char_var + reg_val;
}

/* ========== SUBREG Pattern ========== */
/* Force SUBREG generation through type punning */
typedef struct {
    uint16_t low;
    uint16_t high;
} split32_t;

typedef union {
    uint32_t full;
    split32_t parts;
} reg_union;

NOOPT void test_subreg(void) {
    reg_union u;
    u.full = 0x12345678;
    
    /* Operations on sub-parts should generate SUBREG */
    u.parts.low += 0x1111;   /* SUBREG for low 16 bits */
    u.parts.high -= 0x2222;  /* SUBREG for high 16 bits */
    
    /* Packed struct for SUBREG */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
    } ps;
    
    ps.b = 0x12345678;       /* May generate SUBREG due to misalignment */
    
    global_counter += u.full + ps.b;
}

/* Vector operations for SUBREG */
typedef uint32_t v2u32 __attribute__((vector_size(8)));
typedef uint16_t v4u16 __attribute__((vector_size(8)));

NOOPT void test_subreg_vector(void) {
    v2u32 vec32 = {0x12345678, 0x9ABCDEF0};
    v4u16 vec16;
    
    /* Type punning between vector types */
    memcpy(&vec16, &vec32, sizeof(vec16));
    
    /* Element access may generate SUBREG */
    vec16[0] += 0x1111;
    vec16[3] -= 0x2222;
    
    global_counter += vec16[0] + vec16[3];
}

/* ========== MEM_P with Complex Addressing ========== */
struct nested {
    int data[16];
    struct nested *next;
};

struct complex_mem {
    int array[10][20][30];
    struct nested chain[5];
};

NOOPT void test_complex_mem(void) {
    volatile struct complex_mem cm;
    volatile int *volatile ptrs[10];
    
    /* Complex multi-dimensional array access */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 15; k++) {
                /* Complex addressing expression */
                cm.array[i][j][k] = i * j * k;
                global_counter += cm.array[i+1][j+2][k+3];
            }
        }
    }
    
    /* Chain of pointer dereferences */
    struct nested *current = &cm.chain[0];
    for (int i = 0; i < 4; i++) {
        current->next = &cm.chain[i+1];
        current = current->next;
    }
    
    /* Complex pointer arithmetic */
    current = &cm.chain[0];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            current->data[j] = i * 100 + j;
            global_counter += *(current->data + j * 2);  /* Complex address */
        }
        current = current->next;
    }
    
    /* Inline assembly with memory operand */
    int dummy = 0;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $100, (%%eax, %%eax, 4)"  /* Complex addressing in asm */
        : "=m" (dummy)
        : "r" (&global_counter)
        : "%eax", "memory", "cc"
    );
}

/* ========== Combined Test Function ========== */
/* Function that combines multiple patterns */
NOOPT void test_combined(void) {
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int bits : 10;
    } bf;
    bf.bits = 0x3FF;
    
    /* STRICT_LOW_PART via byte assignment */
    volatile uint32_t val = 0x87654321;
    volatile uint8_t low_byte = (uint8_t)val;
    
    /* SUBREG via union */
    union {
        uint32_t dword;
        uint16_t words[2];
    } u;
    u.dword = 0x12345678;
    u.words[0] = 0xABCD;
    
    /* Complex MEM access */
    volatile int arr[10][10];
    for (int i = 0; i < 9; i++) {
        arr[i][i+1] = arr[i+1][i] + 1;
    }
    
    global_counter += bf.bits + low_byte + u.dword + arr[5][5];
}

/* ========== Main Function ========== */
int main(void) {
    /* Initialize to prevent constant propagation */
    global_counter = 1;
    
    /* Execute all test patterns */
    test_zero_extract();
    test_zero_extract_builtin();
    test_strict_low_part();
    test_subreg();
    test_subreg_vector();
    test_complex_mem();
    test_combined();
    
    /* Dummy computation to ensure code isn't eliminated */
    volatile int result = 0;
    for (int i = 0; i < 100; i++) {
        result += global_counter * i;
    }
    
    return result == 0 ? 0 : 0;  /* Always return 0, but prevent optimization */
}
