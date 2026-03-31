/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

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
    
    /* Use volatile to prevent constant folding */
    volatile uint32_t input = seed + 1;
    u.full = input;
    
    /* These should generate ZERO_EXTRACT RTL */
    uint32_t extracted1 = u.bits.mid;        /* Extract middle 12 bits */
    uint32_t extracted2 = u.bits.high;       /* Extract high 12 bits */
    
    /* Combine with masking to ensure extraction patterns */
    uint32_t result = (extracted1 << 4) | (extracted2 & 0xFF);
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART operations */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    /* Structure with small integer for low-part access */
    struct {
        uint8_t low_byte;
        uint8_t pad[3];
        uint32_t full_word;
    } s;
    
    volatile int temp = seed + 2;
    
    /* This should generate STRICT_LOW_PART RTL */
    s.low_byte = temp & 0xFF;  /* Only modify low 8 bits */
    
    /* Another pattern using pointer to low part */
    uint32_t* ptr = &s.full_word;
    *ptr = temp;  /* Full word assignment */
    
    /* Then modify only low part through different type */
    uint8_t* low_ptr = (uint8_t*)ptr;
    low_ptr[0] = (temp >> 8) & 0xFF;  /* STRICT_LOW_PART pattern */
    
    use_int(s.full_word);
    return s.full_word;
}

/* Pattern 3: Generate SUBREG and complex MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    /* Array for memory access patterns */
    int32_t array[16];
    volatile int index = (seed + 3) & 0xF;
    volatile int value = seed + 4;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 100 + seed;
    }
    
    /* Complex memory addressing with type punning */
    
    /* Pattern 3a: SUBREG through pointer arithmetic */
    short* short_ptr = (short*)((char*)array + index * sizeof(int32_t));
    *short_ptr = value & 0xFFFF;  /* MEM with SUBREG access */
    
    /* Pattern 3b: Different type access to same memory */
    uint8_t* byte_ptr = (uint8_t*)&array[5];
    byte_ptr[1] = (value >> 8) & 0xFF;  /* Another SUBREG pattern */
    
    /* Pattern 3c: Nested memory references */
    int** ptr_array = (int**)array;
    if (index < 8) {
        /* Complex addressing mode */
        int* indirect = (int*)((char*)array + index * 2);
        ptr_array[0] = indirect;
    }
    
    use_ptr(ptr_array);
    
    /* Return checksum of array */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += array[i];
    }
    return sum;
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int selector = seed & 3;
    int result = 0;
    
    /* Switch with different pattern combinations */
    switch (selector) {
        case 0: {
            /* ZERO_EXTRACT + STRICT_LOW_PART combination */
            union {
                uint64_t full;
                struct {
                    uint32_t low;
                    uint32_t high;
                } parts;
            } u;
            
            u.full = (uint64_t)seed << 16;
            /* Extract 20 bits from middle */
            uint32_t extracted = (u.parts.low >> 8) & 0xFFFFF;
            
            /* Store to low part of structure */
            struct {
                uint32_t data;
            } s;
            s.data = extracted;  /* Full assignment */
            
            /* Then modify low 16 bits only */
            uint16_t* low_ptr = (uint16_t*)&s.data;
            low_ptr[0] = extracted & 0xFFFF;  /* STRICT_LOW_PART */
            
            result = s.data;
            break;
        }
            
        case 1: {
            /* MEM_P with complex addressing + SUBREG */
            int buffer[8];
            volatile int offset = (seed >> 4) & 0x7;
            
            /* Fill buffer */
            for (int i = 0; i < 8; i++) {
                buffer[i] = seed + i * 1000;
            }
            
            /* Access through computed pointer with type change */
            char* base = (char*)buffer;
            int32_t* int_ptr = (int32_t*)(base + offset * 2);
            
            /* SUBREG access to part of the word */
            int16_t* short_view = (int16_t*)int_ptr;
            short_view[1] = seed & 0x7FFF;  /* Modify high 16 bits of 32-bit word */
            
            result = buffer[offset];
            break;
        }
            
        case 2: {
            /* Nested extractions */
            uint32_t val = seed ^ 0xABCDEF;
            
            /* Multiple ZERO_EXTRACT patterns */
            uint32_t ext1 = (val >> 4) & 0xFFF;      /* 12 bits */
            uint32_t ext2 = (val >> 16) & 0xFFFF;    /* 16 bits */
            uint32_t ext3 = val & 0x7;               /* 3 bits */
            
            /* Combine with STRICT_LOW_PART store */
            struct {
                uint32_t field;
            } container;
            
            container.field = ext1;
            /* Modify only portion */
            uint8_t* byte_view = (uint8_t*)&container.field;
            byte_view[2] = ext2 & 0xFF;  /* STRICT_LOW_PART of middle byte */
            
            result = container.field + ext3;
            break;
        }
            
        default: {
            /* Loop with mixed patterns */
            int accum = 0;
            for (int i = 0; i < 4; i++) {
                volatile int iter_val = seed + i;
                
                /* ZERO_EXTRACT in loop */
                uint32_t bits = (iter_val >> (i * 2)) & ((1 << (8 - i)) - 1);
                
                /* Store with STRICT_LOW_PART */
                uint32_t storage;
                uint8_t* stor_bytes = (uint8_t*)&storage;
                stor_bytes[i % 4] = bits & 0xFF;
                
                accum += storage;
            }
            result = accum;
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Complex control flow with all patterns */
__attribute__((noinline))
static int pattern_complex_flow(void) {
    volatile int control = seed;
    int result = 0;
    
    /* Conditional with different RTL patterns */
    if (control & 1) {
        /* ZERO_EXTRACT dominant path */
        union {
            uint32_t word;
            struct {
                uint16_t low;
                uint16_t high;
            } halves;
        } u;
        
        u.word = control;
        result = u.halves.low | ((u.halves.high & 0xF) << 16);
    } else {
        /* STRICT_LOW_PART dominant path */
        uint32_t temp = control ^ 0x87654321;
        uint16_t* ptr = (uint16_t*)&temp;
        
        /* Modify low part */
        ptr[0] = (control >> 8) & 0xFFFF;  /* STRICT_LOW_PART */
        result = temp;
    }
    
    /* Loop with MEM_P patterns */
    int array[4];
    for (int i = 0; i < 4; i++) {
        volatile int idx = (control >> (i * 3)) & 0x3;
        
        /* Complex addressing */
        int* elem = &array[idx];
        
        /* Access through different-sized views */
        if (i & 1) {
            uint8_t* byte_elem = (uint8_t*)elem;
            byte_elem[1] = (result >> (i * 2)) & 0xFF;  /* SUBREG */
        } else {
            *elem = result + i;  /* Full MEM */
        }
    }
    
    /* Final checksum */
    for (int i = 0; i < 4; i++) {
        result += array[i];
    }
    
    use_int(result);
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Execute all patterns */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_complex_flow();
    
    /* Additional mixed pattern in main */
    volatile int main_seed = seed;
    
    /* Quick ZERO_EXTRACT in main */
    uint32_t quick_extract = (main_seed >> 12) & 0xFFF;
    
    /* Quick STRICT_LOW_PART in main */
    uint32_t storage;
    uint8_t* bytes = (uint8_t*)&storage;
    bytes[0] = quick_extract & 0xFF;
    
    checksum ^= storage;
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* External function definitions (in separate file normally) */
void use_int(int x) {
    /* Prevent optimization */
    asm volatile("" : : "r"(x));
}

void use_short(short x) {
    asm volatile("" : : "r"(x));
}

void use_ptr(void* x) {
    asm volatile("" : : "r"(x));
}

void use_long(long x) {
    asm volatile("" : : "r"(x));
}
