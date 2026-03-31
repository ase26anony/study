/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile seed to prevent constant folding */
static volatile int seed = 0x12345678;

/* Pattern 1: Generate ZERO_EXTRACT RTL */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Union with bitfields for ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low : 8;
            uint32_t mid : 8;
            uint32_t high : 16;
        } bits;
    } u;
    
    volatile uint32_t input = seed + 1;
    u.full = input;
    
    /* Multiple extractions to increase chances */
    int result = 0;
    result += u.bits.low;          /* Should generate ZERO_EXTRACT for low 8 bits */
    result += u.bits.mid << 8;     /* Should generate ZERO_EXTRACT for middle 8 bits */
    result += u.bits.high << 16;   /* Should generate ZERO_EXTRACT for high 16 bits */
    
    /* Additional extraction using masking */
    uint32_t mask = 0x0000FF00;
    result += (u.full & mask) >> 8;  /* Alternative pattern */
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    struct {
        uint8_t low_byte;
        uint8_t mid_byte;
        uint16_t high_word;
    } s;
    
    volatile uint32_t input = seed + 2;
    
    /* Assignments to partial registers */
    s.low_byte = input & 0xFF;           /* Should generate STRICT_LOW_PART */
    s.mid_byte = (input >> 8) & 0xFF;    /* Another partial assignment */
    
    /* Pointer-based partial assignment */
    uint16_t *ptr = &s.high_word;
    *ptr = (input >> 16) & 0xFFFF;       /* Should generate STRICT_LOW_PART for 16-bit */
    
    /* Union-based partial modification */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } u;
    u.full = 0;
    u.bytes[1] = (input >> 8) & 0xFF;    /* Partial store through union */
    
    use_short(s.high_word);
    return s.low_byte + s.mid_byte + s.high_word + u.bytes[1];
}

/* Pattern 3: Generate SUBREG and complex MEM_P RTL */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    /* Array with type punning */
    uint32_t array[16];
    volatile int index = seed % 12;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = seed + i;
    }
    
    /* Complex memory addressing with SUBREG */
    uint16_t *short_ptr = (uint16_t*)((char*)array + index);
    *short_ptr = seed & 0xFFFF;          /* Should generate SUBREG + MEM */
    
    /* Different type access to same memory */
    uint8_t *byte_ptr = (uint8_t*)&array[2];
    byte_ptr[1] = (seed >> 8) & 0xFF;    /* Another SUBREG pattern */
    
    /* Nested SUBREG through pointer arithmetic */
    int32_t *int_ptr = &array[4];
    int_ptr += index % 4;                /* Pointer arithmetic */
    *int_ptr = seed;                     /* MEM with complex address */
    
    use_ptr(array);
    return array[0] + *short_ptr + byte_ptr[1] + *int_ptr;
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int selector = seed % 4;
    int result = 0;
    
    /* Control flow to generate different RTL contexts */
    switch (selector) {
        case 0: {
            /* ZERO_EXTRACT in loop */
            union {
                uint32_t val;
                struct {
                    uint32_t a : 3;
                    uint32_t b : 5;
                    uint32_t c : 24;
                } fields;
            } u;
            u.val = seed;
            
            for (int i = 0; i < 3; i++) {
                result += u.fields.a << i;
                result += u.fields.b >> i;
            }
            result += u.fields.c;
            break;
        }
            
        case 1: {
            /* STRICT_LOW_PART with conditionals */
            struct {
                uint16_t low;
                uint16_t high;
            } s;
            
            volatile uint32_t input = seed;
            if (input & 1) {
                s.low = input & 0xFFFF;      /* STRICT_LOW_PART */
            } else {
                s.high = (input >> 16) & 0xFFFF; /* Another STRICT_LOW_PART */
            }
            result = s.low + s.high;
            break;
        }
            
        case 2: {
            /* SUBREG and MEM in loop */
            uint32_t buffer[8];
            volatile int idx = seed % 6;
            
            for (int i = 0; i < 8; i++) {
                buffer[i] = seed + i * 100;
            }
            
            /* Access through different-sized types */
            uint8_t *byte_view = (uint8_t*)buffer;
            uint16_t *short_view = (uint16_t*)buffer;
            
            byte_view[idx] = seed & 0xFF;
            short_view[idx + 1] = (seed >> 8) & 0xFFFF;
            
            result = buffer[0] + buffer[1];
            break;
        }
            
        case 3: {
            /* All patterns combined */
            union {
                uint32_t full;
                uint8_t parts[4];
            } u;
            u.full = seed;
            
            struct {
                uint8_t a;
                uint8_t b;
            } s;
            
            s.a = u.parts[0];          /* Partial assignment */
            s.b = u.parts[1] & 0x7F;   /* Partial assignment with mask */
            
            uint32_t mem[4];
            uint16_t *mem_ptr = (uint16_t*)mem;
            mem_ptr[1] = s.a + s.b;    /* MEM + SUBREG */
            
            result = u.full + s.a + s.b + mem[0];
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Nested patterns for deep recursion */
__attribute__((noinline))
static int pattern_nested(void) {
    /* Complex structure with bitfields */
    struct {
        union {
            struct {
                uint32_t low : 10;
                uint32_t mid : 10;
                uint32_t high : 12;
            } bits;
            uint32_t full;
        } u;
        uint8_t bytes[8];
    } data;
    
    volatile uint32_t input = seed;
    data.u.full = input;
    
    /* Extract and store partial results */
    uint32_t temp = data.u.bits.mid;          /* ZERO_EXTRACT */
    
    /* Store to partial memory location */
    uint16_t *ptr = (uint16_t*)&data.bytes[2];
    *ptr = temp & 0xFFFF;                     /* STRICT_LOW_PART + MEM */
    
    /* Further processing with SUBREG */
    uint8_t *byte_ptr = (uint8_t*)ptr;
    byte_ptr[1] = (temp >> 8) & 0xFF;         /* Another SUBREG */
    
    use_long(data.u.full + *ptr + byte_ptr[1]);
    return data.u.bits.low + data.u.bits.high + *ptr;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute all patterns */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_nested();
    
    /* Use volatile to ensure execution */
    volatile int final_result = checksum;
    printf("Checksum: %d\n", final_result);
    
    return final_result & 0xFF;
}

/* Dummy external function definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
