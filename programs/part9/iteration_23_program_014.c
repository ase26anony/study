/* Comprehensive test for GCC RTL resource tracking patterns */
#include <stdint.h>
#include <string.h>

/* Test 1: Bit-field operations for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int flag:1;
    volatile unsigned int value:10;
    unsigned int padding:21;
    volatile unsigned int mode:4;
    unsigned int :0; /* force alignment */
};

struct nested_bitfield {
    struct {
        volatile unsigned int a:3;
        volatile unsigned int b:5;
    } inner;
    volatile unsigned int c:8;
};

void test_bitfields(void) {
    struct bitfield_struct bs = {0};
    struct nested_bitfield nb = {0};
    
    /* Simple bit-field assignments */
    bs.flag = 1;
    bs.value = 511; /* Max for 10 bits */
    bs.mode = 7;
    
    /* Cross assignments */
    unsigned int temp = bs.value;
    bs.flag = temp & 1;
    
    /* Nested bit-field access */
    nb.inner.a = 5;
    nb.inner.b = nb.inner.a + 1;
    nb.c = nb.inner.b << 2;
    
    /* Complex expression with bit-fields */
    bs.value = (bs.flag ? 100 : 200) + nb.c;
}

/* Test 2: Partial register operations for STRICT_LOW_PART */
void test_partial_registers(void) {
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    volatile int vi = 1000;
    
    /* Casts to smaller types */
    vs1 = (short)vi + 50;
    vc1 = (char)(vi / 10);
    
    /* Arithmetic on sub-word types */
    vs2 = vs1 * 2;
    vc2 = vc1 + 32;
    
    /* Store partial results */
    vi = (int)vs1 + (int)vc1;
    
    /* Mixed operations */
    vs1 = (short)((vi & 0xFF) + vc2);
    
#ifdef __i386__
    /* x86-specific partial register patterns */
    asm volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (vs1)
        : "r" (vs2)
        : "ax"
    );
#endif
    
#ifdef __arm__
    /* ARM-specific partial register patterns */
    asm volatile (
        "uxth %0, %1\n\t"
        "add %0, %0, #50"
        : "=r" (vs1)
        : "r" (vi)
    );
#endif
}

/* Test 3: Sub-register accesses for SUBREG */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

union type_punning {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint8_t bytes[4];
};

void test_subregisters(void) {
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3;
    
    /* Vector element access */
    int element = v1[2];
    v1[1] = element + 10;
    
    /* Vector arithmetic */
    v3 = v1 + v2;
    element = v3[0] + v3[3];
    
    /* Type punning through union */
    union type_punning pun;
    pun.full = 0x12345678;
    pun.parts.low = pun.parts.high + 1;
    pun.bytes[0] = pun.bytes[3];
    
    /* Mixed-size operations */
    v8hi vh = {1, 2, 3, 4, 5, 6, 7, 8};
    short sh = vh[3];
    vh[4] = (short)element;
    
    /* Float/integer conversions */
    volatile float f = 3.14159f;
    volatile double d = 2.71828;
    uint32_t fi = *(uint32_t*)&f;
    uint64_t di = *(uint64_t*)&d;
    
    f = *(float*)&fi;
    d = *(double*)&di;
}

/* Test 4: Combined patterns */
struct combined {
    volatile unsigned int bits:4;
    volatile short partial;
    volatile int full;
};

void test_combined_patterns(void) {
    struct combined c = {0};
    volatile int temp;
    
    /* Bit-field to partial register */
    c.bits = 7;
    c.partial = (short)c.bits * 100;
    
    /* Partial register to bit-field */
    temp = c.partial;
    c.bits = temp & 0xF;
    
    /* Complex nested expression */
    c.full = (c.bits << 16) | (c.partial & 0xFFFF);
    
    /* Loop with combined patterns */
    for (int i = 0; i < 10; i++) {
        c.bits = (c.bits + 1) & 0xF;
        c.partial = (short)(c.partial + c.bits);
        c.full += c.partial;
    }
    
    /* Conditional with mixed patterns */
    if (c.full > 1000) {
        c.bits = 0;
        c.partial = -1;
    } else {
        c.bits = 15;
        c.partial = 0;
    }
}

/* Test 5: Builtin functions for bit manipulation */
void test_builtins(void) {
    volatile unsigned int x = 0x12345678;
    volatile unsigned int y = 0x9ABCDEF0;
    volatile int result;
    
    /* Bit counting builtins */
    result = __builtin_popcount(x);
    result += __builtin_clz(y);
    result += __builtin_ctz(x | 1);
    result += __builtin_parity(y);
    
    /* Bit extraction */
    result = __builtin_extract(x, 4, 8); /* Extract 8 bits starting at bit 4 */
    
    /* Bit reversal */
    result = __builtin_bitreverse32(x);
    
    /* Rotate operations */
    result = __builtin_rotateleft32(x, 5);
    result += __builtin_rotateright32(y, 3);
}

/* Test 6: Memory operations with volatile */
void test_volatile_memops(void) {
    volatile struct {
        unsigned int a:2;
        unsigned int b:6;
        unsigned int c:8;
    } mem = {0};
    
    volatile short mem_array[10];
    volatile char char_array[20];
    
    /* Volatile bit-field memory access */
    mem.a = 1;
    mem.b = mem.a * 10;
    mem.c = mem.b + 50;
    
    /* Array operations with partial types */
    for (int i = 0; i < 10; i++) {
        mem_array[i] = (short)(i * 100);
        char_array[i * 2] = (char)(mem_array[i] & 0xFF);
    }
    
    /* Mixed volatile accesses */
    unsigned int combined = (mem.c << 16) | (mem_array[5] & 0xFFFF);
    mem.a = combined & 0x3;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int test_selector = 0;
    
    /* Use command line or environment to select tests */
    if (argc > 1) {
        test_selector = argv[1][0] - '0';
    }
    
    /* Execute tests based on selector */
    switch (test_selector) {
        case 0:
            test_bitfields();
            test_partial_registers();
            test_subregisters();
            test_combined_patterns();
            test_builtins();
            test_volatile_memops();
            break;
        case 1:
            test_bitfields();
            break;
        case 2:
            test_partial_registers();
            break;
        case 3:
            test_subregisters();
            break;
        case 4:
            test_combined_patterns();
            break;
        case 5:
            test_builtins();
            break;
        case 6:
            test_volatile_memops();
            break;
        default:
            /* Run all tests in sequence */
            for (int i = 0; i < 10; i++) {
                test_bitfields();
                test_partial_registers();
                test_subregisters();
                test_combined_patterns();
            }
            break;
    }
    
    /* Ensure program has observable output */
    volatile int checksum = 0;
    checksum += test_selector;
    
    return checksum & 0xFF;
}
