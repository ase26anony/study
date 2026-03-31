/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using volatile bit-fields to force ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT RTL */
    bf.low_bits = 0xAB;           /* Writing to bit-field within larger int */
    bf.middle_bits = 0xCDEF;      /* Another bit-field write */
    bf.high_bit = 1;              /* Single bit write */
    
    /* Force compiler to actually generate the code */
    volatile int sink = 0;
    sink = bf.low_bits + bf.middle_bits + bf.high_bit;
}

/* Alternative using __builtin_bitfield */
NOOPT void test_zero_extract_builtin(void) {
    volatile unsigned int value = 0x12345678;
    
    /* Using builtins that might generate ZERO_EXTRACT */
    unsigned int extracted = __builtin_bitfield_extract(value, 8, 8);
    __builtin_bitfield_insert(value, 0xFF, 16, 8);
    
    volatile int sink = extracted + value;
}

/* ===== STRICT_LOW_PART Pattern ===== */
/* Using inline assembly with %L0 modifier on x86 */
NOOPT void test_strict_low_part(void) {
    volatile unsigned short low_part;
    volatile unsigned int full_reg;
    
    /* Force partial register update */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (low_part)
        : "r" ((unsigned short)0x1234)
        : "cc"
    );
    
    /* Another approach: char assignment to volatile */
    volatile char byte_var;
    volatile int int_var = 0x12345678;
    
    /* This should generate STRICT_LOW_PART for byte store */
    byte_var = (char)int_var;
    
    /* Use inline assembly with explicit low-part constraint */
    unsigned int reg;
    asm volatile (
        "movl $0x12345678, %0\n\t"
        "movb $0xAA, %b0\n\t"  /* %b0 accesses low byte */
        : "=r" (reg)
        :
        : "cc"
    );
    
    volatile int sink = low_part + byte_var + reg;
}

/* ===== SUBREG Pattern ===== */
/* Using packed structures and type punning */
struct __attribute__((packed)) packed_struct {
    char a;
    short b;
    int c;
};

NOOPT void test_subreg(void) {
    struct packed_struct ps;
    ps.a = 0x11;
    ps.b = 0x2233;
    ps.c = 0x44556677;
    
    /* Accessing misaligned short should generate SUBREG */
    volatile short misaligned_short = ps.b;
    
    /* Type punning via union */
    union {
        uint32_t full;
        uint16_t halves[2];
    } u;
    u.full = 0x12345678;
    
    /* Accessing half should generate SUBREG */
    volatile uint16_t half = u.halves[0];
    
    /* Vector operations */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    
    /* Extracting element might use SUBREG */
    volatile int element = vec[2];
    
    volatile int sink = misaligned_short + half + element;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
NOOPT void test_complex_mem(void) {
    volatile int array[100][100];
    volatile int *ptr_array[50];
    
    /* Initialize to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        ptr_array[i] = &array[i*2][0];
    }
    
    /* Complex addressing modes */
    volatile int result = 0;
    
    /* Multi-dimensional array with index calculation */
    result += array[10][20];
    result += array[result % 50][result % 25];
    
    /* Pointer chain with offset */
    result += *(ptr_array[10] + 15);
    result += *(*(&ptr_array[5]) + 3);
    
    /* Structure with pointer arithmetic */
    struct complex {
        int data[10][10];
        int *extra;
    } cs;
    
    cs.extra = &array[0][0];
    result += cs.data[5][5];
    result += *(cs.extra + 100);
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        : 
        : "r" (&array[0][0])
        : "eax", "ebx", "memory"
    );
    
    volatile int sink = result;
}

/* ===== Combined Test Function ===== */
/* Function that combines multiple patterns */
NOOPT void test_combined(void) {
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field : 4;
    } bf;
    bf.field = 7;
    
    /* STRICT_LOW_PART via byte store */
    volatile char byte_store;
    volatile int int_val = 0x12345678;
    byte_store = (char)int_val;
    
    /* SUBREG via packed access */
    struct __attribute__((packed)) {
        char a;
        short b;
    } packed;
    packed.a = 1;
    volatile short subreg_access = packed.b;
    
    /* MEM_P with complex addressing */
    volatile int multi_array[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            multi_array[i][j] = i + j;
        }
    }
    volatile int complex_mem = multi_array[5][5] + multi_array[byte_store % 10][subreg_access % 10];
    
    /* Use all results to prevent elimination */
    volatile int sink = bf.field + byte_store + subreg_access + complex_mem;
}

/* ===== Main Function ===== */
int main(void) {
    /* Call all test functions */
    test_zero_extract();
    test_zero_extract_builtin();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined();
    
    /* Additional volatile operations to ensure code isn't optimized away */
    volatile int dummy = 0;
    for (int i = 0; i < 100; i++) {
        dummy += i;
    }
    
    return dummy == 0 ? 0 : 1;
}
