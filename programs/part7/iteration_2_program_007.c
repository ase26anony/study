/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile variables to prevent constant folding */
static volatile int g_volatile_int = 0x12345678;
static volatile short g_volatile_short = 0xABCD;
static volatile char g_volatile_char = 0x42;
static volatile int g_volatile_index = 3;

/* Pattern 1: Generate ZERO_EXTRACT operations */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Method 1: Using bitfield union */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 8;
            uint32_t high: 16;
        } bits;
    } u;
    
    u.full = g_volatile_int;
    int result1 = u.bits.high;  /* Should generate ZERO_EXTRACT */
    
    /* Method 2: Using shift and mask with volatile */
    volatile uint32_t v = g_volatile_int;
    int result2 = (v >> 16) & 0xFFFF;  /* Alternative ZERO_EXTRACT pattern */
    
    /* Method 3: Extract from memory */
    volatile uint32_t mem[4] = {0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x87654321};
    uint32_t val = mem[g_volatile_index & 3];
    int result3 = (val >> 8) & 0xFF;  /* ZERO_EXTRACT from memory */
    
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return result1 + result2 + result3;
}

/* Pattern 2: Generate STRICT_LOW_PART operations */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    /* Method 1: Structure with small members */
    struct {
        char low_byte;
        char pad[3];
        int full_word;
    } s;
    
    volatile int temp = g_volatile_int;
    s.low_byte = temp & 0xFF;  /* Should generate STRICT_LOW_PART */
    
    /* Method 2: Pointer to low part */
    int value = g_volatile_int;
    char *low_ptr = (char*)&value;
    *low_ptr = g_volatile_char;  /* Modify only low byte */
    
    /* Method 3: Union assignment */
    union {
        int32_t i;
        struct {
            int16_t low;
            int16_t high;
        } parts;
    } u;
    
    u.i = g_volatile_int;
    u.parts.low = g_volatile_short;  /* STRICT_LOW_PART of 16-bit */
    
    use_int(s.low_byte);
    use_int(value);
    use_short(u.parts.low);
    
    return s.low_byte + value + u.i;
}

/* Pattern 3: Generate SUBREG and complex MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    /* Create array with complex access patterns */
    int32_t array[16];
    volatile int idx = g_volatile_index;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 0x11111111;
    }
    
    /* SUBREG pattern 1: Access as smaller type */
    int32_t *int_ptr = &array[idx & 15];
    int16_t *short_ptr = (int16_t*)int_ptr;
    *short_ptr = g_volatile_short;  /* SUBREG store */
    
    /* SUBREG pattern 2: Byte access with offset */
    char *byte_ptr = (char*)array + (idx * 3);
    int32_t *aliased_int = (int32_t*)byte_ptr;
    *aliased_int = g_volatile_int;  /* MEM with complex address */
    
    /* Complex MEM_P with address computation */
    int32_t *complex_ptr = array + (idx << 1) + (idx >> 1);
    use_ptr(complex_ptr);
    
    /* Nested SUBREG/MEM pattern */
    struct {
        int32_t a;
        int16_t b;
        int32_t c;
    } nested;
    
    nested.a = g_volatile_int;
    int16_t *b_ptr = &nested.b;
    *b_ptr = g_volatile_short;  /* SUBREG within MEM */
    
    /* Return checksum of array */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += array[i];
    }
    
    return sum + nested.a + *b_ptr;
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int selector = g_volatile_int;
    int result = 0;
    
    /* Complex control flow with pattern generation */
    for (int i = 0; i < 4; i++) {
        switch ((selector >> (i * 4)) & 0xF) {
            case 0: {
                /* ZERO_EXTRACT in loop */
                union {
                    uint64_t full;
                    struct {
                        uint32_t low;
                        uint32_t high;
                    } words;
                } u;
                u.full = (uint64_t)g_volatile_int << 32 | g_volatile_int;
                result += u.words.high;  /* ZERO_EXTRACT */
                break;
            }
            case 1: {
                /* STRICT_LOW_PART in loop */
                int32_t val = g_volatile_int + i;
                int8_t *low = (int8_t*)&val;
                *low = i;  /* STRICT_LOW_PART */
                result += val;
                break;
            }
            case 2: {
                /* SUBREG/MEM in loop */
                int32_t buffer[8];
                int16_t *ptr = (int16_t*)((char*)buffer + i * 2);
                *ptr = g_volatile_short + i;  /* SUBREG store */
                result += buffer[i/2];
                break;
            }
            default: {
                /* Combined pattern */
                struct {
                    int32_t data;
                    int16_t low;
                } s;
                s.data = g_volatile_int;
                s.low = (selector >> 8) & 0xFFFF;  /* ZERO_EXTRACT + STRICT_LOW_PART */
                result += s.data;
                break;
            }
        }
    }
    
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    
    printf("Checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy definitions for external functions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
