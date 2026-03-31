/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
    volatile int input = seed + 1;
    int result = 0;
    
    /* Method 1: Using bitfields in union */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 12;
            uint32_t high: 12;
        } bits;
    } u;
    
    u.full = input;
    result = u.bits.mid;  /* Should generate ZERO_EXTRACT for 12-bit field at offset 8 */
    
    /* Method 2: Manual masking and shifting with volatile */
    volatile uint32_t mask = 0x0FFF0000;
    volatile int shift = 16;
    result += (input & mask) >> shift;
    
    /* Method 3: Nested bitfield extraction */
    struct nested {
        struct {
            unsigned a: 3;
            unsigned b: 5;
            unsigned c: 24;
        } inner;
    } n;
    
    n.inner.c = input;
    result += n.inner.b;  /* Extract 5-bit field */
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART operations */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    volatile int input = seed + 2;
    int result = 0;
    
    /* Method 1: Structure with small integer type */
    struct partial {
        char low_byte;
        char mid_byte;
        int full;
    } p;
    
    p.full = input;
    p.low_byte = (input >> 8) & 0xFF;  /* Modify only low part of structure member */
    
    /* Method 2: Pointer to partial type */
    short *short_ptr = (short*)&p.full;
    *short_ptr = input & 0xFFFF;  /* Modify low 16 bits only */
    
    /* Method 3: Union with different sized members */
    union {
        int32_t i32;
        int16_t i16[2];
        int8_t i8[4];
    } u;
    
    u.i32 = input;
    u.i16[0] = (input >> 16) & 0xFFFF;  /* STRICT_LOW_PART on 16-bit subreg */
    
    /* Complex control flow to keep RTL alive */
    for (volatile int i = 0; i < 3; i++) {
        if (i & 1) {
            u.i8[1] = (input >> 8) & 0xFF;
        } else {
            u.i8[3] = (input >> 24) & 0xFF;
        }
    }
    
    result = u.i32;
    use_int(result);
    return result;
}

/* Pattern 3: Generate SUBREG and complex MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    volatile int index = seed & 0xF;
    volatile int value = seed + 3;
    int result = 0;
    
    /* Create array with complex access patterns */
    int array[32] __attribute__((aligned(16)));
    
    /* Initialize array */
    for (volatile int i = 0; i < 32; i++) {
        array[i] = i * 7;
    }
    
    /* Method 1: SUBREG through pointer casting */
    short *short_ptr = (short*)((char*)array + index * sizeof(int));
    *short_ptr = value & 0xFFFF;  /* MEM with SUBREG access */
    
    /* Method 2: Different type access with offset */
    char *char_base = (char*)array;
    int32_t *int_ptr = (int32_t*)(char_base + 8 + index * 2);
    *int_ptr = value;  /* MEM with complex address calculation */
    
    /* Method 3: Nested structure with array */
    struct nested_array {
        int header;
        short data[20];
        int footer;
    } na;
    
    na.header = value;
    volatile int idx2 = (index * 3) % 20;
    na.data[idx2] = (value >> 8) & 0x7FFF;  /* MEM with SUBREG */
    
    /* Method 4: Pointer arithmetic with multiple types */
    void *base_ptr = &array[16];
    int8_t *byte_ptr = (int8_t*)base_ptr;
    byte_ptr += (index * 3) & 0x1F;
    
    int16_t *final_ptr = (int16_t*)byte_ptr;
    *final_ptr = value & 0xFF;  /* Complex MEM access */
    
    /* Compute result from all operations */
    result = array[index] + *int_ptr + na.data[idx2] + *final_ptr;
    use_int(result);
    return result;
}

/* Pattern 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int input = seed + 4;
    volatile int selector = seed & 0x3;
    int result = 0;
    
    /* Mixed structure for combined patterns */
    struct mixed {
        union {
            uint32_t full;
            struct {
                uint16_t low;
                uint16_t high;
            } parts;
        } u;
        char bytes[8];
        short words[4];
    } m;
    
    m.u.full = input;
    
    /* Switch statement with different pattern combinations */
    switch (selector) {
        case 0:
            /* ZERO_EXTRACT + STRICT_LOW_PART */
            m.u.parts.low = (input >> 4) & 0x0FFF;  /* 12-bit extract into 16-bit field */
            m.bytes[2] = m.u.parts.low & 0xFF;      /* Strict low part of byte */
            break;
            
        case 1:
            /* SUBREG + MEM_P with pointer arithmetic */
            short *word_ptr = (short*)(m.bytes + 1);
            *word_ptr = input & 0xFFFF;  /* Unaligned MEM access with SUBREG */
            break;
            
        case 2:
            /* Nested combinations */
            m.words[1] = (input >> 8) & 0xFF;  /* STRICT_LOW_PART */
            result = m.u.parts.high;           /* ZERO_EXTRACT */
            break;
            
        case 3:
            /* Complex memory addressing */
            int *alias_ptr = (int*)&m.words[selector];
            *alias_ptr = input;  /* MEM with potential SUBREG */
            break;
    }
    
    /* Loop with varying patterns */
    for (volatile int i = 0; i < 4; i++) {
        if (i & 1) {
            /* ZERO_EXTRACT pattern */
            result += (m.u.full >> (i * 4)) & 0xF;
        } else {
            /* STRICT_LOW_PART pattern */
            m.bytes[i] = (input >> (i * 8)) & 0xFF;
        }
    }
    
    /* Final MEM access with address computation */
    volatile int offset = (input & 0x7) * 2;
    char *final_base = (char*)&m;
    int16_t *final_access = (int16_t*)(final_base + offset);
    result += *final_access;
    
    use_int(result);
    return result;
}

/* Pattern 5: Recursive-like patterns with function pointers */
__attribute__((noinline))
static int pattern_complex_flow(void) {
    volatile int base = seed + 5;
    int result = base;
    
    /* Array of different sized elements */
    struct varying {
        int32_t i32;
        int16_t i16;
        int8_t i8;
        int32_t i32_2;
    } var[4];
    
    /* Initialize with pattern */
    for (volatile int i = 0; i < 4; i++) {
        var[i].i32 = base + i * 17;
        var[i].i16 = (base + i * 13) & 0xFFFF;
        var[i].i8 = (base + i * 11) & 0xFF;
        var[i].i32_2 = base - i * 19;
    }
    
    /* Conditional access patterns */
    volatile int mode = base & 0x3;
    
    if (mode == 0) {
        /* ZERO_EXTRACT from structure array */
        result = var[0].i16 + var[1].i8;
    } else if (mode == 1) {
        /* STRICT_LOW_PART to structure members */
        var[2].i8 = (base >> 4) & 0x0F;
        var[3].i16 = base & 0x7FFF;
        result = var[2].i32 + var[3].i32_2;
    } else {
        /* MEM_P with complex addressing */
        int *ptr = (int*)((char*)var + sizeof(struct varying) * (mode - 1));
        *ptr = base * 3;
        result = *ptr;
    }
    
    /* Nested loop with mixed operations */
    for (volatile int i = 0; i < 2; i++) {
        for (volatile int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if ((i + j) & 1) {
                /* Access as different type */
                short *sptr = (short*)&var[idx];
                result += sptr[1];  /* SUBREG access */
            } else {
                /* Bitfield extraction */
                result += (var[idx].i32 >> 8) & 0xFF;
            }
        }
    }
    
    use_int(result);
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Execute all patterns */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_complex_flow();
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile int final_check = checksum;
    for (volatile int i = 0; i < 10; i++) {
        final_check = (final_check * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Final checksum: %d (0x%08x)\n", final_check, final_check);
    
    return final_check != 0 ? 0 : 1;
}

/* External function definitions (should be in separate file for best results) */
void use_int(int x) {
    /* Empty function - linker will resolve */
    static volatile int sink;
    sink = x;
}

void use_short(short x) {
    static volatile short sink;
    sink = x;
}

void use_char(char x) {
    static volatile char sink;
    sink = x;
}

void use_ptr(void* x) {
    static volatile void* sink;
    sink = x;
}
