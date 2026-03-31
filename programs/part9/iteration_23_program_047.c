/* Comprehensive test for GCC RTL resource tracking coverage */
#include <stdint.h>
#include <string.h>

/* Test 1: Bit-field operations for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int flag:1;
    unsigned int value:10;
    unsigned int pad:21;
    volatile unsigned int mode:4;
    unsigned int :0; /* force alignment */
};

struct nested_bitfield {
    struct {
        volatile unsigned int a:3;
        unsigned int b:5;
    } inner;
    volatile unsigned int c:8;
};

void test_bitfields(void) {
    struct bitfield_struct s1 = {0};
    struct nested_bitfield s2 = {0};
    volatile int result;
    
    /* Various bit-field assignments */
    s1.flag = 1;
    s1.value = 511; /* max 10-bit value */
    s1.mode = 7;
    
    s2.inner.a = 3;
    s2.inner.b = 20;
    s2.c = 255;
    
    /* Cross assignments */
    result = s1.flag;
    s1.flag = s2.inner.a & 1;
    result = s1.value + s2.c;
    
    /* Complex bit-field expression */
    s1.mode = (s1.value >> 5) & 0xF;
}

/* Test 2: Partial register operations for STRICT_LOW_PART */
void test_partial_registers(void) {
    volatile short vs;
    volatile char vc;
    volatile int vi = 100;
    volatile long vl = 1000;
    
    /* Direct assignments to partial types */
    vs = (short)vi + 5;
    vc = (char)(vi * 2);
    
    /* Arithmetic with partial types */
    vs = vs * 2;
    vc = vc + 1;
    
    /* Mixed-size operations */
    vi = vs + vc;
    vs = (short)(vl >> 2);
    
    /* Pointer dereference with partial types */
    short arr[10];
    volatile short *ps = arr;
    *ps = (short)vi;
    vc = (char)(*ps);
}

/* Test 3: Sub-register accesses for SUBREG */
typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));

union mixed_types {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint8_t bytes[4];
};

void test_subreg(void) {
    /* Vector operations */
    v4si v = {1, 2, 3, 4};
    v8hi vh = {1, 2, 3, 4, 5, 6, 7, 8};
    volatile int element;
    
    element = v[2];
    v[0] = element + 1;
    
    /* Vector element as partial type */
    volatile short selem = vh[3];
    vh[4] = (short)(element + selem);
    
    /* Union type punning */
    union mixed_types u;
    u.full = 0x12345678;
    volatile uint16_t low_part = u.parts.low;
    volatile uint8_t byte_part = u.bytes[1];
    
    u.parts.high = low_part + 1;
    u.bytes[2] = byte_part * 2;
    
    /* Float/integer conversions */
    volatile float f = 3.14f;
    volatile double d = 2.71828;
    uint32_t fi;
    uint64_t di;
    
    memcpy(&fi, &f, sizeof(fi));
    memcpy(&di, &d, sizeof(di));
    
    /* Access partial views of floats */
    volatile uint16_t f_low = (uint16_t)(fi & 0xFFFF);
    fi = (fi & 0xFFFF0000) | f_low;
}

/* Test 4: Combined patterns */
struct combined {
    volatile unsigned int field1:5;
    volatile unsigned int field2:11;
    volatile short partial;
    uint32_t full;
};

void test_combined(void) {
    struct combined c = {0};
    volatile int temp;
    
    /* Bit-field to partial register */
    c.partial = (short)c.field1;
    
    /* Partial register to bit-field */
    c.field2 = c.partial & 0x7FF;
    
    /* Complex expression with both */
    temp = c.field1 + c.partial;
    c.field2 = (temp >> 3) & 0x7FF;
    
    /* Through union with bit-fields */
    union {
        struct {
            volatile unsigned int a:4;
            volatile unsigned int b:4;
        } bits;
        volatile uint8_t byte;
    } u;
    
    u.bits.a = 5;
    u.bits.b = 10;
    c.partial = u.byte * 2;
}

/* Test 5: Architecture-specific patterns */
#ifdef __i386__
void test_x86_specific(void) {
    volatile uint32_t result;
    volatile uint16_t w1 = 1000, w2 = 2000;
    volatile uint8_t b1 = 100, b2 = 200;
    
    /* These may generate partial register ops on x86 */
    result = w1 + w2;
    result = b1 * b2;
    
    /* Inline assembly that might use partial registers */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw %2, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (w1)
        : "r" (w2), "r" (w1)
        : "ax"
    );
}
#endif

#ifdef __arm__
void test_arm_specific(void) {
    volatile uint32_t val32;
    volatile uint16_t val16;
    
    /* ARM may use STRICT_LOW_PART for 16-bit stores */
    val32 = 0x12345678;
    val16 = (uint16_t)val32;
    
    /* Byte extraction */
    volatile uint8_t val8 = (uint8_t)(val32 >> 16);
}
#endif

/* Test 6: Builtin functions for bit manipulation */
void test_builtins(void) {
    volatile unsigned int x = 0x12345678;
    volatile int count;
    
    /* These may generate ZERO_EXTRACT patterns */
    count = __builtin_popcount(x);
    count = __builtin_ctz(x);
    count = __builtin_parity(x);
    
    /* Bit extraction builtins */
    count = __builtin_extract_return_addr(&x);
}

/* Test 7: Loop and conditional patterns */
void test_loops(void) {
    struct bitfield_struct arr[10];
    volatile short partial_arr[10];
    volatile int i, sum = 0;
    
    for (i = 0; i < 10; i++) {
        arr[i].flag = i & 1;
        arr[i].value = i * 10;
        arr[i].mode = i & 0xF;
        
        /* Cross assignment in loop */
        partial_arr[i] = (short)arr[i].value;
        
        /* Conditional with bit-field */
        if (arr[i].flag) {
            sum += arr[i].value;
        }
        
        /* Complex expression */
        arr[i].mode = (partial_arr[i] >> 2) & 0xF;
    }
    
    /* Prevent optimization */
    volatile int dummy = sum;
    (void)dummy;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int test_num = 0;
    
    /* Use argc to make control flow non-deterministic to compiler */
    if (argc > 1) {
        test_num = argv[1][0] - '0';
    }
    
    /* Execute tests based on input (prevents dead code elimination) */
    switch (test_num & 7) {
        case 0: test_bitfields(); break;
        case 1: test_partial_registers(); break;
        case 2: test_subreg(); break;
        case 3: test_combined(); break;
        case 4: 
            #ifdef __i386__
            test_x86_specific();
            #endif
            break;
        case 5: 
            #ifdef __arm__
            test_arm_specific();
            #endif
            break;
        case 6: test_builtins(); break;
        case 7: test_loops(); break;
    }
    
    /* Ensure all tests are referenced to prevent elimination */
    void (*tests[])(void) = {
        test_bitfields,
        test_partial_registers,
        test_subreg,
        test_combined,
        #ifdef __i386__
        test_x86_specific,
        #endif
        #ifdef __arm__
        test_arm_specific,
        #endif
        test_builtins,
        test_loops
    };
    
    /* Simple computation to ensure program runs */
    volatile int result = 0;
    for (volatile int i = 0; i < 10; i++) {
        result += i * 2;
    }
    
    return result > 0 ? 0 : 1;
}
