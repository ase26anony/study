/* Test program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile short g_volatile_short = 0;
volatile char g_volatile_char = 0;

/* ===== 1. Bit-field patterns for ZERO_EXTRACT ===== */

struct BitFieldStruct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    unsigned int padding:21;
};

struct NestedBitField {
    struct {
        volatile unsigned int low:4;
        volatile unsigned int high:4;
    } nibbles;
    volatile unsigned int full:8;
};

void test_bitfield_extract(void) {
    struct BitFieldStruct s1 = {0};
    struct BitFieldStruct s2 = {0};
    
    /* Direct bit-field assignments */
    s1.flag = 1;
    s1.value = 511; /* Max 10-bit value */
    
    /* Cross assignments between bit-fields */
    s2.value = s1.value;
    s2.flag = s1.flag;
    
    /* Complex expression with bit-fields */
    unsigned int temp = (s1.value << 1) | s1.flag;
    s2.value = temp & 0x3FF; /* Mask to 10 bits */
    
    /* Store in volatile to ensure side effects */
    g_volatile_int = s2.value;
}

void test_nested_bitfield(void) {
    struct NestedBitField nb = {0};
    
    /* Access nested bit-fields */
    nb.nibbles.low = 0xF;
    nb.nibbles.high = 0xA;
    nb.full = (nb.nibbles.high << 4) | nb.nibbles.low;
    
    /* Extract partial bit-fields */
    unsigned int low_part = nb.nibbles.low;
    unsigned int high_part = nb.nibbles.high;
    
    g_volatile_int = low_part + high_part;
}

/* ===== 2. Partial register patterns for STRICT_LOW_PART ===== */

void test_partial_register(void) {
    volatile short vs = 0;
    volatile char vc = 0;
    
    /* Casts to smaller types */
    int large_val = 0x12345678;
    vs = (short)large_val;  /* Should generate partial register update */
    vc = (char)large_val;
    
    /* Arithmetic on sub-word types */
    short s1 = 1000;
    short s2 = 2000;
    vs = s1 + s2;  /* May generate STRICT_LOW_PART for 16-bit result */
    
    /* Compound assignment on partial types */
    char c = 50;
    c += 30;  /* Update of char variable */
    vc = c;
    
    /* Store to global volatile */
    g_volatile_short = vs;
    g_volatile_char = vc;
}

/* Architecture-specific partial register patterns */
#ifdef __i386__
void test_x86_partial_reg(void) {
    volatile short s_result;
    volatile char c_result;
    
    /* Inline assembly that might trigger partial register handling */
    __asm__ volatile (
        "movw $0x1234, %0\n\t"
        "movb $0x56, %1"
        : "=r"(s_result), "=r"(c_result)
        :
        : "memory"
    );
    
    g_volatile_short = s_result;
    g_volatile_char = c_result;
}
#endif

#ifdef __arm__
void test_arm_partial_reg(void) {
    volatile short s_result;
    
    /* ARM may generate partial register ops for 16-bit loads */
    __asm__ volatile (
        "movw %0, #0x1234"
        : "=r"(s_result)
        :
        : "memory"
    );
    
    g_volatile_short = s_result;
}
#endif

/* ===== 3. Sub-register patterns for SUBREG ===== */

/* Vector type for SUBREG patterns */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

void test_vector_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Access individual elements - should generate SUBREG */
    int elem2 = vec_int[2];
    short elem5 = vec_short[5];
    
    /* Type punning through union */
    union {
        v4si vec;
        int array[4];
    } u;
    
    u.vec = vec_int;
    int via_array = u.array[1];  /* Different access path */
    
    /* Mixed-type operations */
    vec_short[0] = (short)vec_int[0];
    
    g_volatile_int = elem2 + elem5 + via_array;
}

/* Packed structure for SUBREG patterns */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

void test_packed_struct(void) {
    struct PackedStruct ps;
    
    /* Access misaligned members - may generate SUBREG */
    ps.a = 0x11;
    ps.b = 0x22334455;  /* May be misaligned due to packed attribute */
    ps.c = 0x6677;
    
    /* Read through different type */
    char *ptr = (char*)&ps;
    int extracted = *(int*)(ptr + 1);  /* Type punning */
    
    g_volatile_int = extracted;
}

/* ===== 4. Combined patterns ===== */

struct Combined {
    volatile unsigned int bits:12;
    volatile short half;
    volatile char byte;
};

void test_combined_patterns(void) {
    struct Combined c = {0};
    
    /* Bit-field to partial register */
    c.bits = 0xABC;
    c.half = (short)c.bits;  /* ZERO_EXTRACT -> SUBREG/STRICT_LOW_PART */
    
    /* Partial register to bit-field */
    c.byte = 0x7F;
    c.bits = c.byte;  /* May involve multiple conversions */
    
    /* Complex expression */
    unsigned int temp = c.bits;
    temp = (temp << 4) | (c.byte & 0xF);
    c.half = (short)(temp & 0xFFFF);
    
    /* Loop to ensure code isn't optimized away */
    for (int i = 0; i < 3; i++) {
        c.bits += i;
        c.half -= i;
        c.byte ^= i;
    }
    
    g_volatile_int = c.bits + c.half + c.byte;
}

/* Use builtins that may generate extract patterns */
void test_builtin_extract(void) {
    unsigned int x = 0x12345678;
    
    /* Builtins that work with bits */
    int count = __builtin_popcount(x);  /* May use bit extraction */
    int parity = __builtin_parity(x);
    int leading_zeros = __builtin_clz(x);
    
    /* Extract specific bits */
    unsigned int extracted = (x >> 8) & 0xFF;  /* Manual extraction */
    
    g_volatile_int = count + parity + leading_zeros + extracted;
}

/* ===== Main test driver ===== */

typedef void (*test_func_t)(void);

/* Array of test functions */
test_func_t test_functions[] = {
    test_bitfield_extract,
    test_nested_bitfield,
    test_partial_register,
    test_vector_subreg,
    test_packed_struct,
    test_combined_patterns,
    test_builtin_extract,
#ifdef __i386__
    test_x86_partial_reg,
#endif
#ifdef __arm__
    test_arm_partial_reg,
#endif
    NULL
};

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Use command line or volatile to control which tests run */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    } else {
        /* Use volatile to prevent compile-time optimization */
        volatile int choice = g_volatile_int;
        test_to_run = choice % 8;
    }
    
    /* Run specific test or all tests */
    if (test_to_run == 0) {
        /* Run all tests */
        for (int i = 0; test_functions[i] != NULL; i++) {
            test_functions[i]();
        }
    } else {
        /* Run specific test */
        int idx = (test_to_run - 1) % 8;
        if (test_functions[idx] != NULL) {
            test_functions[idx]();
        }
    }
    
    /* Final computation to ensure program does something */
    int result = g_volatile_int + g_volatile_short + g_volatile_char;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
