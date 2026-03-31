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
            uint32_t low: 8;
            uint32_t mid: 8;
            uint32_t high: 16;
        } bits;
    } u;
    
    /* Volatile input prevents constant propagation */
    volatile uint32_t input = seed + 1;
    u.full = input;
    
    /* These operations should generate ZERO_EXTRACT */
    uint32_t result = 0;
    result |= (u.bits.high << 16);  /* Extract high 16 bits */
    result |= (u.bits.mid << 8);    /* Extract middle 8 bits */
    result |= u.bits.low;           /* Extract low 8 bits */
    
    /* Complex extraction with masking */
    uint32_t masked = u.full;
    masked &= 0x00FF00FF;           /* Alternate bytes */
    masked |= ((u.full & 0xFF00FF00) >> 8);
    
    /* Use results to prevent dead code elimination */
    use_int(result);
    use_int(masked);
    
    return (result ^ masked) & 0xFFFF;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    /* Structure with small members for partial updates */
    struct {
        uint8_t a;
        uint8_t b;
        uint16_t c;
        uint32_t d;
    } s = {0};
    
    volatile int temp = seed + 2;
    
    /* These assignments should generate STRICT_LOW_PART */
    s.a = temp & 0xFF;              /* Modify only low 8 bits */
    s.b = (temp >> 8) & 0xFF;       /* Modify next 8 bits */
    s.c = temp & 0xFFFF;            /* Modify low 16 bits */
    
    /* Pointer-based partial modification */
    uint32_t* ptr = &s.d;
    *ptr = (*ptr & 0xFFFF0000) | (temp & 0xFFFF);  /* Modify only low half */
    
    /* Union type punning for partial access */
    union {
        uint32_t full;
        uint16_t halves[2];
    } u;
    u.full = temp * 3;
    u.halves[0] = temp & 0xABCD;    /* Modify only low 16 bits */
    
    use_int(s.a + s.b + s.c);
    use_int(u.full);
    
    return s.d ^ u.full;
}

/* Pattern 3: Generate SUBREG and MEM_P RTL */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    /* Array for complex memory addressing */
    int array[32];
    volatile int index = seed % 16;
    volatile int value = seed * 5;
    
    /* Initialize array */
    for (int i = 0; i < 32; i++) {
        array[i] = i * i;
    }
    
    /* Type punning through pointers - should generate SUBREG */
    short* short_ptr = (short*)((char*)array + index * sizeof(int));
    *short_ptr = value & 0xFFFF;    /* Store as short through int pointer */
    
    /* Different type access with offset */
    char* char_ptr = (char*)array;
    char_ptr[index + 4] = (value >> 8) & 0xFF;
    
    /* Complex addressing mode */
    int* complex_ptr = &array[(index * 3) % 32];
    *complex_ptr = (*complex_ptr & 0xFF00FF00) | (value & 0x00FF00FF);
    
    /* Nested memory access */
    int** ptr_to_ptr = &complex_ptr;
    **ptr_to_ptr = **ptr_to_ptr ^ value;
    
    use_ptr(array);
    use_int(*complex_ptr);
    
    return array[index] ^ array[(index + 1) % 32];
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int selector = seed % 4;
    int result = 0;
    
    /* Different control flow paths */
    switch (selector) {
        case 0: {
            /* ZERO_EXTRACT in loop */
            union {
                uint64_t full;
                struct {
                    uint32_t low;
                    uint32_t high;
                } parts;
            } u;
            u.full = seed * 7;
            
            for (int i = 0; i < 4; i++) {
                /* Extract different bit ranges */
                uint32_t extracted = (u.parts.high >> (i * 4)) & 0xF;
                result += extracted;
            }
            break;
        }
            
        case 1: {
            /* STRICT_LOW_PART with conditional */
            struct {
                uint32_t data;
                uint16_t low;
                uint16_t high;
            } s;
            s.data = seed * 11;
            
            if (seed & 1) {
                s.low = (seed >> 4) & 0xFFFF;  /* Partial update */
            } else {
                s.high = (seed >> 8) & 0xFFFF; /* Partial update */
            }
            result = s.data;
            break;
        }
            
        case 2: {
            /* SUBREG/MEM with pointer arithmetic */
            int buffer[8];
            volatile int offset = seed % 4;
            
            /* Access through different type pointers */
            for (int i = 0; i < 8; i++) {
                buffer[i] = i * 100;
            }
            
            /* Type-punned access */
            short* sp = (short*)buffer;
            sp[offset * 2] = seed & 0x7FFF;
            sp[offset * 2 + 1] = (seed >> 16) & 0x7FFF;
            
            result = buffer[offset];
            break;
        }
            
        case 3: {
            /* All patterns combined */
            union {
                uint32_t val;
                uint8_t bytes[4];
            } u;
            u.val = seed * 13;
            
            /* ZERO_EXTRACT through bitfield */
            uint32_t extracted = (u.val >> 8) & 0xFF;
            
            /* STRICT_LOW_PART through pointer */
            uint16_t* hp = (uint16_t*)&u.val;
            hp[0] = extracted * 3;  /* Modify low 16 bits */
            
            /* MEM access with computation */
            int temp[2] = {u.val, extracted};
            int* tp = &temp[(seed >> 2) & 1];
            result = *tp;
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Nested structures with bitfields */
__attribute__((noinline))
static int pattern_nested_bitfields(void) {
    /* Complex nested structure with bitfields */
    struct inner {
        unsigned a: 3;
        unsigned b: 5;
        unsigned c: 8;
        unsigned d: 16;
    };
    
    struct outer {
        struct inner part1;
        struct inner part2;
        uint32_t full;
    } o;
    
    volatile uint32_t v = seed * 17;
    
    /* Multiple ZERO_EXTRACT operations */
    o.part1.a = v & 0x7;
    o.part1.b = (v >> 3) & 0x1F;
    o.part1.c = (v >> 8) & 0xFF;
    o.part1.d = (v >> 16) & 0xFFFF;
    
    /* STRICT_LOW_PART through pointer to bitfield */
    struct inner* ip = &o.part2;
    ip->a = (v >> 4) & 0x7;
    ip->b = (v >> 9) & 0x1F;
    
    /* MEM access to structure member */
    o.full = o.part1.d | (o.part2.c << 16);
    
    use_int(o.full);
    return o.full ^ (o.part1.a + o.part1.b);
}

/* Main function to execute all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute all patterns */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_nested_bitfields();
    
    /* Additional volatile operations to prevent optimization */
    volatile int final = checksum;
    for (int i = 0; i < 10; i++) {
        final ^= (seed >> i) & 1;
    }
    
    printf("Checksum: %d\n", final);
    printf("Pattern generation complete.\n");
    
    return final & 0xFF;
}

/* Dummy external function definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
