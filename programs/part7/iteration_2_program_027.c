/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_char(char);
extern void use_ptr(void*);

/* Volatile seed to prevent constant propagation */
static volatile int seed = 0x12345678;

/* Pattern 1: Generate ZERO_EXTRACT operations */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Union with bitfields for ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 12;
            uint32_t high: 12;
        } bits;
    } u;
    
    volatile uint32_t input = seed;
    u.full = input;
    
    /* Multiple extractions to increase chances */
    uint32_t result = 0;
    result |= u.bits.low;          /* Should generate ZERO_EXTRACT */
    result |= (u.bits.mid << 8);   /* Another extraction */
    result |= (u.bits.high << 20); /* And another */
    
    /* Complex extraction with masking */
    uint32_t masked = (input >> 4) & 0xFFF; /* 12-bit extraction */
    result ^= masked;
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART operations */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    volatile int input = seed + 1;
    int result = 0;
    
    /* Structure with small members for partial updates */
    struct {
        char byte1;
        char byte2;
        short word;
        int dword;
    } s;
    
    /* Initialize */
    s.dword = input;
    
    /* Partial updates that may generate STRICT_LOW_PART */
    s.byte1 = (input >> 8) & 0xFF;  /* Update only low byte through struct */
    s.word = input & 0xFFFF;        /* Update only low word */
    
    /* Pointer-based partial update */
    char *ptr = (char*)&result;
    ptr[0] = input & 0xFF;          /* Update single byte */
    ptr[1] = (input >> 8) & 0xFF;   /* Update another byte */
    
    /* Union for type punning partial updates */
    union {
        int32_t full;
        struct {
            int16_t low;
            int16_t high;
        } parts;
    } u;
    
    u.full = input;
    u.parts.low = (input >> 16) & 0xFFFF; /* Partial update */
    
    result = s.dword ^ u.full;
    use_int(result);
    return result;
}

/* Pattern 3: Generate SUBREG and complex MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    volatile int index = seed & 0xF;  /* 0-15 range */
    volatile int value = seed + 2;
    
    /* Array with different access patterns */
    int32_t array[32];
    for (int i = 0; i < 32; i++) {
        array[i] = i * 3;
    }
    
    int result = 0;
    
    /* Complex memory addressing with pointer arithmetic */
    char *base = (char*)array;
    
    /* Access through different types (SUBREG generation) */
    int16_t *short_ptr = (int16_t*)(base + index * sizeof(int32_t));
    result += *short_ptr;  /* MEM access with SUBREG */
    
    /* More complex addressing mode */
    int32_t *int_ptr = (int32_t*)(base + (index * 2) + 4);
    result += *int_ptr;
    
    /* Type punning with union for SUBREG */
    union {
        int64_t big;
        int32_t parts[2];
    } u;
    
    u.big = (int64_t)value * 3;
    result += u.parts[0];  /* Access low part */
    result += u.parts[1];  /* Access high part */
    
    /* Pointer to volatile for MEM_P with complex address */
    volatile int32_t *volatile_ptr = (volatile int32_t*)(array + index);
    result += *volatile_ptr;
    
    use_int(result);
    return result;
}

/* Pattern 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int selector = seed & 0x3;
    int result = 0;
    
    /* Switch with different pattern combinations */
    switch (selector) {
        case 0: {
            /* ZERO_EXTRACT + MEM combination */
            union {
                uint32_t val;
                struct {
                    uint32_t a: 5;
                    uint32_t b: 10;
                    uint32_t c: 17;
                } bits;
            } u;
            
            volatile uint32_t input = seed + 3;
            u.val = input;
            
            /* Store extracted bits to memory */
            int32_t mem[4];
            mem[0] = u.bits.a;
            mem[1] = u.bits.b;
            mem[2] = u.bits.c;
            
            /* Access through different pointer types */
            char *cptr = (char*)mem;
            result = cptr[selector];
            break;
        }
            
        case 1: {
            /* STRICT_LOW_PART + SUBREG combination */
            struct {
                int16_t low;
                int16_t high;
            } s;
            
            volatile int input = seed + 4;
            s.low = input & 0xFFFF;  /* Partial update */
            
            /* Access through different views */
            int32_t *as_int = (int32_t*)&s;
            result = *as_int;  /* SUBREG access */
            break;
        }
            
        case 2: {
            /* All three patterns combined */
            volatile int base = seed + 5;
            
            /* ZERO_EXTRACT */
            union {
                uint32_t full;
                uint16_t halves[2];
            } u;
            u.full = base;
            
            /* STRICT_LOW_PART through pointer */
            uint8_t *byte_ptr = (uint8_t*)&u.full;
            byte_ptr[0] = (base >> 8) & 0xFF;
            
            /* MEM with complex address */
            int32_t buffer[8];
            int32_t *ptr = buffer + (base & 0x7);
            *ptr = u.halves[0];  /* Store with possible SUBREG */
            
            result = buffer[base & 0x7];
            break;
        }
            
        default: {
            /* Loop with mixed patterns */
            int32_t temp = 0;
            for (int i = 0; i < 4; i++) {
                volatile int iter_val = seed + i;
                
                /* Alternate between patterns */
                if (i & 1) {
                    /* ZERO_EXTRACT in loop */
                    temp |= (iter_val >> (i * 3)) & 0x7;
                } else {
                    /* Partial update in loop */
                    uint8_t *p = (uint8_t*)&temp;
                    p[i % 4] = iter_val & 0xFF;
                }
            }
            result = temp;
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Nested structures with bitfields and arrays */
__attribute__((noinline))
static int pattern_nested(void) {
    /* Complex nested structure */
    struct inner {
        char a;
        char b;
        int16_t c;
    };
    
    struct outer {
        struct inner part1;
        struct inner part2;
        uint32_t flags: 16;
        uint32_t count: 8;
        uint32_t reserved: 8;
        int32_t data[4];
    };
    
    volatile struct outer obj;
    
    /* Initialize with volatile values */
    volatile int v = seed + 6;
    obj.part1.a = v & 0xFF;
    obj.part1.c = (v >> 8) & 0xFFFF;
    obj.flags = (v >> 16) & 0xFFFF;
    obj.count = (v >> 24) & 0xFF;
    
    /* Access through multiple indirections */
    char *char_ptr = (char*)&obj;
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        result += char_ptr[i];
    }
    
    /* Array access with type conversion */
    int16_t *short_ptr = (int16_t*)obj.data;
    result += short_ptr[1];  /* SUBREG + MEM */
    
    /* Bitfield extraction */
    result |= (obj.flags & 0xF) << 4;
    
    use_int(result);
    return result;
}

/* Main function that runs all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run each pattern */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_nested();
    
    /* Additional volatile operations to prevent optimization */
    volatile int final = checksum;
    printf("Final checksum: %d (0x%08x)\n", final, final);
    
    return final != 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
/* These would normally be in a separate file */
void __attribute__((weak)) use_int(int x) { (void)x; }
void __attribute__((weak)) use_short(short x) { (void)x; }
void __attribute__((weak)) use_char(char x) { (void)x; }
void __attribute__((weak)) use_ptr(void* x) { (void)x; }
