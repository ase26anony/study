/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile variables to prevent constant folding */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 3;
static volatile short volatile_short = 0xABCD;
static volatile char volatile_char = 0x42;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Pattern 1: Union with bitfields */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 12;
            uint32_t high: 12;
        } bits;
    } u;
    
    u.full = volatile_seed;
    int result1 = u.bits.mid;  /* Should generate ZERO_EXTRACT */
    
    /* Pattern 2: Manual masking and shifting */
    uint32_t val = volatile_seed;
    int result2 = (val >> 8) & 0xFFF;  /* Another ZERO_EXTRACT candidate */
    
    /* Pattern 3: Nested extraction */
    struct {
        struct {
            unsigned a: 4;
            unsigned b: 4;
            unsigned c: 8;
        } inner;
        unsigned d: 16;
    } nested;
    
    *(uint32_t*)&nested = volatile_seed;
    int result3 = nested.inner.c;
    
    /* Combine results with external call */
    use_int(result1 + result2 + result3);
    return result1 ^ result2 ^ result3;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    /* Pattern 1: Structure with small members */
    struct {
        char low_byte;
        int rest;
    } s1;
    
    int temp = volatile_seed;
    s1.low_byte = temp & 0xFF;  /* Should generate STRICT_LOW_PART */
    
    /* Pattern 2: Pointer to low part */
    uint32_t data = volatile_seed;
    uint8_t *low_ptr = (uint8_t*)&data;
    *low_ptr = volatile_char;  /* Modify only low byte */
    
    /* Pattern 3: Union assignment to partial register */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } u64;
    
    u64.full = (uint64_t)volatile_seed << 32;
    u64.parts.low = volatile_seed & 0xFFFF;  /* Modify low 32-bit part */
    
    /* Use results */
    use_int(s1.low_byte + *low_ptr + (int)u64.parts.low);
    return data;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    /* Array for memory access patterns */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    int result = 0;
    
    /* Pattern 1: SUBREG through pointer casting */
    volatile_index = volatile_index & 0xF;  /* Ensure bounds */
    
    /* Access as different types - should generate SUBREG */
    short *short_ptr = (short*)((char*)array + volatile_index * sizeof(int));
    *short_ptr = volatile_short;
    result += *short_ptr;
    
    /* Pattern 2: Complex MEM address with SUBREG */
    int *ptr = &array[volatile_index];
    long *long_ptr = (long*)ptr;  /* Type punning */
    use_long(*long_ptr);  /* Force MEM with potential SUBREG */
    
    /* Pattern 3: Nested MEM access */
    struct {
        int data[4];
        struct {
            short a;
            short b;
        } shorts[2];
    } complex;
    
    complex.data[0] = volatile_seed;
    complex.shorts[1].b = volatile_short;
    
    /* Access through computed pointer */
    short *computed = &complex.shorts[volatile_index & 1].a;
    result += *computed;
    
    /* Pattern 4: Pointer arithmetic with different types */
    char *base = (char*)array;
    int offset = volatile_index * sizeof(int);
    int *aliased = (int*)(base + offset + 2);  /* Misaligned access */
    result += *aliased & 0xFFFF;  /* Partial read */
    
    use_int(result);
    return result;
}

/* Function 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    int result = 0;
    volatile int control = volatile_seed;
    
    /* Loop with varying patterns */
    for (int i = 0; i < 4; i++) {
        if (control & (1 << i)) {
            /* ZERO_EXTRACT in conditional */
            union {
                uint32_t val;
                struct {
                    uint32_t a: 10;
                    uint32_t b: 10;
                    uint32_t c: 12;
                } fields;
            } u;
            u.val = volatile_seed + i;
            result += u.fields.b;  /* ZERO_EXTRACT */
        } else {
            /* STRICT_LOW_PART in else branch */
            struct {
                unsigned char small;
                unsigned int large;
            } s;
            s.large = volatile_seed;
            s.small = (volatile_char + i) & 0xFF;  /* STRICT_LOW_PART */
            result += s.small;
        }
    }
    
    /* Switch statement with different memory patterns */
    switch (control & 0x3) {
        case 0: {
            /* MEM with SUBREG */
            int buffer[8];
            short *sp = (short*)buffer;
            sp[volatile_index & 3] = volatile_short;
            result += sp[1];
            break;
        }
        case 1: {
            /* ZERO_EXTRACT with computation */
            uint32_t x = volatile_seed;
            result += (x >> 16) & 0xFF;  /* ZERO_EXTRACT */
            break;
        }
        case 2: {
            /* Combined SUBREG and MEM */
            long data = volatile_seed;
            int *ip = (int*)&data;
            ip[1] = volatile_seed >> 16;  /* SUBREG store */
            result += ip[0];
            break;
        }
        default: {
            /* Complex addressing */
            struct {
                int a;
                char b[3];
                short c;
            } s;
            s.a = volatile_seed;
            short *cp = (short*)&s.b[1];  /* Misaligned short pointer */
            *cp = volatile_short;
            result += s.c;
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Function 5: Nested patterns with recursion simulation */
__attribute__((noinline))
static int pattern_nested(void) {
    /* Multi-level structure for complex MEM access */
    struct level3 {
        int data;
        short extra;
    };
    
    struct level2 {
        struct level3 l3[2];
        char pad;
    };
    
    struct level1 {
        struct level2 l2;
        int counter;
    };
    
    struct level1 obj;
    obj.counter = volatile_seed;
    obj.l2.l3[volatile_index & 1].data = volatile_seed >> 8;
    obj.l2.l3[0].extra = volatile_short;
    
    /* Access through multiple levels - complex MEM address */
    short *sptr = &obj.l2.l3[1].extra;
    *sptr = volatile_char;  /* STRICT_LOW_PART on short */
    
    /* ZERO_EXTRACT from nested field */
    union {
        struct level1 l1;
        uint32_t raw[sizeof(struct level1)/sizeof(uint32_t)];
    } u;
    u.l1 = obj;
    
    int extracted = (u.raw[1] >> 8) & 0xFFFF;  /* ZERO_EXTRACT */
    
    /* SUBREG access to part of structure */
    char *cptr = (char*)&obj;
    int *misaligned = (int*)(cptr + 3);  /* Force SUBREG */
    int temp = *misaligned & 0xFFFFFF;  /* Partial read */
    
    use_int(extracted + temp + *sptr);
    return extracted ^ temp;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Execute each pattern generator */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_nested();
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile int final = checksum;
    for (int i = 0; i < 4; i++) {
        final ^= volatile_seed >> (i * 8);
    }
    
    printf("Checksum: 0x%08X\n", final);
    
    return final & 0xFF;
}

/* External function definitions (in separate file normally) */
#ifdef DEFINE_EXTERNAL_FUNCTIONS
void use_int(int x) {
    volatile static int sink;
    sink = x;
}

void use_short(short x) {
    volatile static short sink;
    sink = x;
}

void use_ptr(void* x) {
    volatile static void* sink;
    sink = x;
}

void use_long(long x) {
    volatile static long sink;
    sink = x;
}
#endif
