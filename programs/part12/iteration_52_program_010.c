/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Writing to bit-fields should generate ZERO_EXTRACT */
    bf.field1 = 7;           /* 4-bit field */
    bf.field2 = 0xAB;        /* 8-bit field */
    bf.field3 = 0x7FF;       /* 12-bit field */
    bf.field4 = 0xCD;        /* 8-bit field */
    
    /* Read back to prevent elimination */
    global_counter += bf.field1 + bf.field2 + bf.field3 + bf.field4;
    
    /* Alternative: Using __builtin_bitfield operations */
    unsigned int value = 0x12345678;
    unsigned int extracted;
    
    /* Extract bits 8-15 (should generate ZERO_EXTRACT) */
    extracted = __builtin_bitfield_extract(value, 8, 8);
    global_counter += extracted;
    
    /* Insert bits (should also generate ZERO_EXTRACT) */
    value = __builtin_bitfield_insert(value, 0xAA, 16, 8);
    global_counter += value;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val = 0x12345678;
    
    /* Assigning to low parts of variables */
    s_val = (short)i_val;           /* Should generate STRICT_LOW_PART for 16-bit */
    global_counter += s_val;
    
    c_val = (char)i_val;            /* Should generate STRICT_LOW_PART for 8-bit */
    global_counter += c_val;
    
    /* Using inline assembly with low-part modifier on x86 */
    int x = 0xABCD1234;
    int y;
    
    /* Force low-byte operation */
    asm volatile (
        "movb %b1, %b0"
        : "=r" (y)
        : "r" (x)
        : "cc"
    );
    global_counter += y;
    
    /* Force low-word operation */
    asm volatile (
        "movw %w1, %w0"
        : "=r" (y)
        : "r" (x)
        : "cc"
    );
    global_counter += y;
}

/* ===== SUBREG Pattern ===== */
/* Packed structure to force sub-register accesses */
struct __attribute__((packed)) packed_struct {
    char a;
    short b;
    int c;
    char d;
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
    /* Using packed structure */
    struct packed_struct ps;
    ps.a = 1;
    ps.b = 0x1234;
    ps.c = 0x56789ABC;
    ps.d = 0xFF;
    
    /* Accessing members forces SUBREG operations */
    global_counter += ps.a + ps.b + ps.c + ps.d;
    
    /* Using union for type-punning */
    union type_pun pun;
    pun.full = 0x12345678;
    
    /* Accessing parts generates SUBREG */
    global_counter += pun.parts.low;
    global_counter += pun.parts.high;
    
    /* Vector operations with sub-register access */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    
    /* Extracting elements generates SUBREG */
    int elem = vec[2];
    global_counter += elem;
    
    /* Bit manipulation with masking */
    uint32_t val = 0x12345678;
    uint16_t low_half = val & 0xFFFF;      /* Generates SUBREG */
    uint16_t high_half = (val >> 16) & 0xFFFF; /* Generates SUBREG */
    
    global_counter += low_half + high_half;
}

/* ===== MEM_P Pattern with Complex Addressing ===== */
#define ARRAY_SIZE 100

struct complex_struct {
    int data[10][10];
    int extra[5];
};

NOOPT void test_mem_complex_address(void) {
    volatile struct complex_struct cs[ARRAY_SIZE];
    volatile int multi_array[10][10][10];
    volatile int * volatile ptr_array[ARRAY_SIZE];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                cs[i].data[j][k] = i * 100 + j * 10 + k;
            }
        }
        ptr_array[i] = &cs[i].data[0][0];
    }
    
    /* Complex memory addressing patterns */
    
    /* 1. Multi-dimensional array with complex index */
    int sum = 0;
    for (int i = 1; i < 9; i++) {
        for (int j = 1; j < 9; j++) {
            for (int k = 1; k < 9; k++) {
                /* Complex addressing expression */
                sum += multi_array[i][j][k] 
                     + multi_array[i-1][j][k] 
                     + multi_array[i+1][j][k];
            }
        }
    }
    global_counter += sum;
    
    /* 2. Structure pointer chain with offsets */
    struct complex_struct *ptr = &cs[10];
    for (int i = 0; i < 5; i++) {
        /* Complex addressing through structure */
        global_counter += ptr->data[i][i] 
                        + ptr->data[i+1][i] 
                        + ptr->data[i][i+1];
        ptr++;
    }
    
    /* 3. Pointer arithmetic with multiple terms */
    int *base_ptr = &cs[20].data[0][0];
    for (int i = 0; i < 50; i++) {
        /* Complex address: base + index*scale + offset */
        int val = *(base_ptr + i*2 + 5);
        global_counter += val;
    }
    
    /* 4. Volatile memory operations with inline assembly */
    volatile int mem_location = 0;
    asm volatile (
        "movl %1, %0\n\t"
        "addl $100, %0\n\t"
        "movl %0, %1"
        : "+r" (mem_location)
        : "m" (mem_location)
        : "cc"
    );
    global_counter += mem_location;
    
    /* 5. Array of pointers with dereferencing */
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        /* Complex: *(ptr_array[i] + i) */
        global_counter += *(ptr_array[i] + i);
    }
}

/* ===== Combined Test Function ===== */
/* This function combines multiple patterns to maximize coverage */
NOOPT void test_combined_patterns(void) {
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 5;
        volatile unsigned int c : 24;
    } bits;
    
    bits.a = 5;
    bits.b = 0x1F;
    bits.c = 0xFFFFFF;
    global_counter += bits.a + bits.b + bits.c;
    
    /* STRICT_LOW_PART via type conversion */
    volatile int src = 0x89ABCDEF;
    volatile short dst_short = src;  /* Low 16 bits */
    volatile char dst_char = src;    /* Low 8 bits */
    global_counter += dst_short + dst_char;
    
    /* SUBREG via union access */
    union {
        uint64_t quad;
        uint32_t dwords[2];
        uint16_t words[4];
    } u;
    
    u.quad = 0x0123456789ABCDEFull;
    global_counter += u.dwords[0] + u.words[2];
    
    /* MEM_P with complex addressing */
    volatile int arr[4][4] = {{0}};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Complex index calculation */
            global_counter += arr[(i+j)%4][(i*j)%4];
        }
    }
}

/* ===== Main Function ===== */
int main(void) {
    /* Call all test functions multiple times to ensure coverage */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_complex_address();
        test_combined_patterns();
    }
    
    /* Return the global counter to prevent dead code elimination */
    return global_counter == 0 ? 0 : 1;
}
