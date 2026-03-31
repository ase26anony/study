/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 4;
    volatile unsigned int mid_bits : 8;
    volatile unsigned int high_bits : 20;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT for bit-field writes */
    bf.low_bits = 0x5;      /* Writing to bit-field within larger integer */
    bf.mid_bits = 0xAA;     /* Another bit-field write */
    bf.high_bits = 0x12345; /* Larger bit-field */
    
    /* Mix with computation to prevent optimization */
    global_counter += bf.low_bits + bf.mid_bits;
}

/* Alternative using __builtin_bitfield */
NOOPT void test_zero_extract_builtin(void) {
    volatile uint32_t value = 0x12345678;
    
    /* Using __builtin_bitfield to potentially generate ZERO_EXTRACT */
    __builtin_bitfield((value & 0xFFF), 4, 8) = 0xAA;
    
    global_counter += value;
}

/* ========== STRICT_LOW_PART Pattern ========== */
/* Using inline assembly with %L0 modifier on x86 */
NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_val;
    volatile uint32_t int_val = 0xDEADBEEF;
    
    /* Inline assembly that should generate STRICT_LOW_PART */
    asm volatile (
        "movw %w1, %0"  /* Move word (low 16 bits) */
        : "=r" (short_val)
        : "r" (int_val)
        : "memory"
    );
    
    /* Another approach: volatile char assignment */
    volatile char *byte_ptr = (volatile char *)&int_val;
    *byte_ptr = 0x42;  /* Writing low byte */
    
    global_counter += short_val + *byte_ptr;
}

/* ========== SUBREG Pattern ========== */
/* Using type-punning and packed structures */
typedef union {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint8_t bytes[4];
} pun_union;

NOOPT void test_subreg(void) {
    pun_union u;
    u.full = 0x12345678;
    
    /* Operations on sub-parts that should generate SUBREG */
    u.parts.low += 0x100;   /* Accessing low 16 bits */
    u.parts.high -= 0x200;  /* Accessing high 16 bits */
    
    /* Using vector types for SUBREG generation */
    typedef uint32_t v2u16 __attribute__((vector_size(4)));
    v2u16 vec = {0x1111, 0x2222};
    uint16_t elem = vec[0] + vec[1];  /* Extracting elements */
    
    global_counter += u.full + elem;
}

/* ========== MEM_P with Complex Addressing ========== */
/* Complex memory addressing patterns */
struct nested {
    int data[16];
    struct nested *next;
};

NOOPT void test_complex_mem(void) {
    volatile struct nested array[4];
    volatile struct nested *ptr = &array[0];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            array[i].data[j] = i * 100 + j;
        }
        if (i < 3) array[i].next = &array[i + 1];
    }
    
    /* Complex addressing expressions */
    int idx1 = global_counter & 3;
    int idx2 = (global_counter >> 2) & 15;
    
    /* These should generate complex MEM addresses */
    volatile int val1 = array[idx1].data[idx2];
    volatile int val2 = ptr->next->next->data[5];
    volatile int val3 = *(int*)((char*)ptr + sizeof(int) * idx2 + 8);
    
    /* Even more complex with pointer arithmetic */
    volatile int *base = array[0].data;
    volatile int val4 = base[idx1 * 16 + idx2];
    volatile int val5 = *(base + idx1 * 4 + idx2);
    
    global_counter += val1 + val2 + val3 + val4 + val5;
}

/* ========== Combined Test Function ========== */
/* Function that combines all patterns */
NOOPT void test_combined(void) {
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field : 10;
    } bf;
    bf.field = 0x1FF;
    
    /* STRICT_LOW_PART via byte store */
    volatile uint32_t word = 0x87654321;
    *(volatile uint8_t *)&word = 0xAA;
    
    /* SUBREG via union access */
    union {
        uint32_t dword;
        uint16_t words[2];
    } u;
    u.dword = 0x12345678;
    u.words[0] = 0xABCD;
    
    /* Complex MEM access */
    volatile int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 8 + j;
        }
    }
    volatile int val = arr[global_counter & 7][(global_counter >> 3) & 7];
    
    global_counter += bf.field + word + u.dword + val;
}

/* ========== Main Function ========== */
int main(void) {
    /* Call all test functions multiple times with different conditions */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_zero_extract_builtin();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to change behavior in loops */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Dummy return to prevent optimization */
    return global_counter == 0 ? 0 : 1;
}
