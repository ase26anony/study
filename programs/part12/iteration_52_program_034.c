/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our patterns */
#define NO_OPT __attribute__((noinline, noipa))

/* Volatile to force memory operations */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int low_bits : 8;
    unsigned int middle_bits : 16;
    unsigned int high_bit : 1;
    unsigned int padding : 7;
} __attribute__((packed));

volatile struct bitfield_struct bf;

NO_OPT void test_zero_extract(void) {
    /* Writing to bit-fields often generates ZERO_EXTRACT */
    bf.low_bits = 0xAB;          /* Should generate ZERO_EXTRACT for 8-bit field */
    bf.middle_bits = 0xCDEF;     /* Should generate ZERO_EXTRACT for 16-bit field */
    bf.high_bit = 1;             /* Should generate ZERO_EXTRACT for 1-bit field */
    
    /* Alternative: Using __builtin_bitfield */
    unsigned int val = 0;
    __builtin_bitfield((unsigned int)val, 8, 16) = 0x1234;
    
    /* Force usage to prevent elimination */
    global_counter += bf.low_bits + bf.middle_bits;
}

/* ===== STRICT_LOW_PART Pattern ===== */
/* Using inline assembly with %L0 modifier on x86 */
NO_OPT void test_strict_low_part(void) {
    unsigned int x = 0x12345678;
    unsigned short y = 0xABCD;
    unsigned char z = 0xEF;
    
    /* Force partial register updates */
    asm volatile (
        "movw %w1, %0\n\t"        /* Writing 16-bit to 32-bit register */
        : "=r" (x)
        : "r" (y)
        : "memory"
    );
    
    /* Alternative: volatile char assignment to force low-part update */
    volatile unsigned char *ptr = (volatile unsigned char *)&x;
    *ptr = z;                     /* Updates low byte only */
    
    /* Another approach: explicit low-part assembly */
    asm volatile (
        "movb %b1, %b0\n\t"       /* Explicit low byte move */
        : "+r" (x)
        : "r" (z)
        : "memory"
    );
    
    global_counter += x;
}

/* ===== SUBREG Pattern ===== */
/* Using unions and type-punning for SUBREG */
union subreg_union {
    uint32_t full;
    uint16_t half[2];
    uint8_t byte[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

NO_OPT void test_subreg(void) {
    union subreg_union u;
    u.full = 0xDEADBEEF;
    
    /* Operations on sub-parts should generate SUBREG */
    u.parts.low += 0x1111;        /* Accessing 16-bit part of 32-bit */
    u.half[1] = u.half[0] ^ 0x5555; /* SUBREG operations */
    
    /* Using vector types can also generate SUBREG */
    typedef uint32_t v2u16 __attribute__((vector_size(8)));
    v2u16 vec = {0x1234, 0x5678};
    uint16_t element = vec[0];    /* Extracting element may use SUBREG */
    
    /* Packed structure access */
    struct packed {
        uint16_t a;
        uint16_t b;
    } __attribute__((packed)) p = {0x1111, 0x2222};
    uint16_t temp = p.a + p.b;    /* May involve SUBREG for packed access */
    
    global_counter += u.full + element + temp;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
struct complex_mem {
    int data[256][256];
    int more_data[100];
};

NO_OPT void test_complex_mem(void) {
    static volatile struct complex_mem cm;
    volatile int *volatile ptrs[10];
    int indices[5] = {10, 20, 30, 40, 50};
    
    /* Complex addressing modes */
    cm.data[indices[0] + global_counter][indices[1] * 2] = 0x1234;
    cm.data[indices[2]][indices[3] + indices[4]] = 
        cm.data[indices[1]][indices[0]] + 1;
    
    /* Pointer arithmetic with multiple offsets */
    int *base = &cm.more_data[0];
    *(base + indices[0] * 2 + indices[1]) = 0x5678;
    
    /* Multi-dimensional array with computed indices */
    int md_array[32][32];
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            md_array[i][j] = i * j + global_counter;
        }
    }
    
    /* Complex address calculation */
    volatile int result = md_array[indices[0] % 32][indices[1] % 32] +
                         md_array[indices[2] % 32][indices[3] % 32];
    
    global_counter += result;
}

/* ===== Combined Test Function ===== */
NO_OPT void test_combined(void) {
    /* Mix all patterns in one function */
    volatile struct bitfield_struct local_bf;
    
    /* ZERO_EXTRACT */
    local_bf.low_bits = 0x42;
    
    /* STRICT_LOW_PART via inline assembly */
    unsigned int reg = 0;
    unsigned short low_part = 0x8888;
    asm volatile ("movw %w1, %0" : "=r" (reg) : "r" (low_part) : "memory");
    
    /* SUBREG via union */
    union subreg_union u;
    u.full = reg;
    u.parts.high = u.parts.low;
    
    /* MEM_P with complex address */
    static int array[100][100];
    int idx = global_counter % 50;
    array[idx + 10][idx * 2] = u.full;
    
    global_counter += array[idx][idx + 5];
}

/* ===== Main Function ===== */
int main(void) {
    /* Initialize to prevent constant propagation */
    global_counter = 1;
    
    /* Execute all test functions multiple times */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Prevent loop unrolling from simplifying patterns */
        if (global_counter > 1000) {
            global_counter = 1;
        }
    }
    
    /* Dummy computation to ensure code isn't eliminated */
    volatile int result = 0;
    for (int i = 0; i < 100; i++) {
        result += global_counter * i;
    }
    
    return result != 0 ? 0 : 1;
}
