/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations that might eliminate our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */
/* Using bit-fields to trigger ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT for bit-field stores */
    bf.low_bits = 0xAB;          /* 8-bit field */
    bf.middle_bits = 0xCDEF;     /* 16-bit field */
    bf.high_bit = 1;             /* 1-bit field */
    
    /* Complex bit-field operation */
    bf.low_bits = (bf.middle_bits >> 4) & 0x0F;
    
    /* Prevent optimization */
    global_counter += bf.low_bits + bf.high_bit;
}

/* Alternative using __builtin_bitfield */
NOOPT void test_zero_extract_builtin(void) {
    volatile uint32_t value = 0x12345678;
    
    /* Builtin bitfield operations may generate ZERO_EXTRACT */
    __builtin_bitfield_store32(&value, 8, 8, 0xFF);  /* Store 0xFF at bits 8-15 */
    uint32_t extracted = __builtin_bitfield_load32(&value, 16, 8); /* Load bits 16-23 */
    
    global_counter += extracted;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */
NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t byte_var;
    volatile uint32_t int_var = 0x87654321;
    
    /* These should generate STRICT_LOW_PART for partial register updates */
    
    /* Method 1: Direct low-part assignment */
    short_var = (uint16_t)int_var;  /* Low 16 bits */
    
    /* Method 2: Byte assignment */
    byte_var = (uint8_t)(int_var >> 8);
    
    /* Method 3: Inline assembly forcing low-part constraint */
    uint32_t result;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "andl $0xFFFF, %0"
        : "=r" (result)
        : "r" (int_var)
        : "cc"
    );
    
    /* Method 4: Using %b0 for byte register (x86 specific) */
    uint8_t byte_result;
    __asm__ volatile (
        "movb %b1, %b0"
        : "=q" (byte_result)
        : "q" ((uint8_t)int_var)
    );
    
    global_counter += short_var + byte_var + result + byte_result;
}

/* ==================== SUBREG Pattern ==================== */
/* Packed structure to force sub-register accesses */
struct __attribute__((packed)) packed_data {
    uint8_t a;
    uint16_t b;
    uint8_t c;
    uint32_t d;
};

/* Union for type-punning */
union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

NOOPT void test_subreg(void) {
    /* Method 1: Packed structure access */
    struct packed_data pd = {1, 2, 3, 4};
    volatile uint16_t extracted = pd.b;  /* May generate SUBREG */
    
    /* Method 2: Type-punning via union */
    union type_pun pun;
    pun.full = 0xDEADBEEF;
    volatile uint16_t low_part = pun.parts.low;  /* May generate SUBREG */
    
    /* Method 3: Vector operations */
    typedef uint32_t v2u16 __attribute__((vector_size(8)));
    v2u16 vec = {0x1234, 0x5678};
    volatile uint16_t elem = vec[0];  /* May generate SUBREG */
    
    /* Method 4: Bit manipulation with casts */
    uint32_t val = 0xABCD1234;
    volatile uint16_t casted = (uint16_t)val;  /* Low 16 bits */
    
    global_counter += extracted + low_part + elem + casted;
}

/* ==================== MEM_P with Complex Addressing ==================== */
#define ARRAY_SIZE 100

struct nested {
    int data[10];
    struct nested *next;
};

NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int multi_array[10][10][10];
    
    /* Complex addressing expression */
    int i = global_counter % 10;
    int j = (global_counter * 3) % 10;
    int k = (global_counter * 7) % 10;
    
    multi_array[i][j][k] = 42;
    volatile int read = multi_array[k][j][i];  /* Different access pattern */
    
    /* Structure pointer chain */
    struct nested chain[5];
    struct nested *ptr = &chain[0];
    
    /* Initialize chain */
    for (int idx = 0; idx < 5; idx++) {
        for (int d = 0; d < 10; d++) {
            chain[idx].data[d] = idx * 10 + d;
        }
        if (idx < 4) chain[idx].next = &chain[idx + 1];
        else chain[idx].next = NULL;
    }
    
    /* Complex memory access through pointer chain */
    volatile int sum = 0;
    ptr = &chain[0];
    while (ptr) {
        /* Complex addressing: ptr->data[(i + j) % 10] */
        sum += ptr->data[(i + j) % 10];
        ptr = ptr->next;
    }
    
    /* Inline assembly with memory clobber */
    int temp = 0;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (temp)
        : "m" (global_counter)
        : "%eax", "memory"
    );
    
    global_counter += read + sum + temp;
}

/* ==================== Combined Test ==================== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field : 4;
    } bf;
    bf.field = 7;
    
    /* STRICT_LOW_PART via byte store */
    volatile uint32_t big = 0x12345678;
    volatile uint8_t small = (uint8_t)big;
    
    /* SUBREG via packed struct */
    struct __attribute__((packed)) {
        uint8_t a;
        uint16_t b;
    } ps = {1, 2};
    volatile uint16_t sub = ps.b;
    
    /* Complex MEM access */
    volatile int array[10][10];
    int idx1 = global_counter % 10;
    int idx2 = (global_counter * 3) % 10;
    array[idx1][idx2] = 99;
    volatile int val = array[idx2][idx1];
    
    global_counter += bf.field + small + sub + val;
}

/* ==================== Main Function ==================== */
int main(void) {
    /* Initialize global counter */
    global_counter = 1;
    
    /* Execute all pattern tests */
    test_zero_extract();
    test_zero_extract_builtin();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined();
    
    /* Additional iterations to increase coverage probability */
    for (int i = 0; i < 10; i++) {
        test_combined();
        global_counter++;
    }
    
    /* Return something based on computations to prevent optimization */
    return (global_counter > 0) ? 0 : 1;
}
