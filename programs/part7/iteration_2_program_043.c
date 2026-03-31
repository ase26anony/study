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

/* Pattern 1: Generate ZERO_EXTRACT through bitfield operations */
__attribute__((noinline))
int pattern_zero_extract(void) {
    /* Method 1: Union with bitfields */
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
    
    /* Method 2: Manual masking and shifting */
    int temp = volatile_seed;
    int result2 = (temp >> 8) & 0xFFF;  /* Another potential ZERO_EXTRACT */
    
    /* Method 3: Nested extractions */
    int x = volatile_seed;
    int y = (x & 0xFFFF) >> 4;  /* Compound extraction */
    
    /* Use results to keep them alive */
    use_int(result1 + result2 + y);
    return result1 + result2;
}

/* Pattern 2: Generate STRICT_LOW_PART through partial register updates */
__attribute__((noinline))
int pattern_strict_low_part(void) {
    /* Method 1: Structure with small member */
    struct {
        int full;
        char low_byte;
    } s;
    
    s.full = volatile_seed;
    s.low_byte = volatile_char;  /* Should generate STRICT_LOW_PART */
    int result1 = s.full;
    
    /* Method 2: Pointer to low part */
    int value = volatile_seed;
    char *low_ptr = (char*)&value;
    low_ptr[0] = volatile_char;  /* Modify only low byte */
    
    /* Method 3: Union type punning */
    union {
        int32_t i;
        int16_t s[2];
    } u;
    u.i = volatile_seed;
    u.s[0] = volatile_short;  /* Modify low 16 bits */
    
    /* Use results */
    use_int(result1 + value + u.i);
    return result1 + value;
}

/* Pattern 3: Generate SUBREG and complex MEM_P patterns */
__attribute__((noinline))
int pattern_subreg_mem(void) {
    /* Create array with complex access patterns */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Method 1: SUBREG through type punning */
    int base = volatile_seed;
    short *short_ptr = (short*)&base;
    short_ptr[1] = volatile_short;  /* Access high 16 bits as short */
    
    /* Method 2: Complex MEM addressing with pointer arithmetic */
    volatile int idx = volatile_index;
    int *mem_ptr = &array[idx * 2 + 1];
    *mem_ptr = volatile_seed;  /* Complex address calculation */
    
    /* Method 3: Nested MEM access through pointer chains */
    int **ptr_ptr = &mem_ptr;
    **ptr_ptr = volatile_seed + 1;
    
    /* Method 4: Array access with byte offset */
    char *byte_ptr = (char*)array;
    byte_ptr[idx * sizeof(int) + 2] = volatile_char;
    
    /* Use the memory results */
    use_ptr(array);
    return array[0] + array[idx];
}

/* Pattern 4: Combined patterns in complex control flow */
__attribute__((noinline))
int pattern_combined(void) {
    int result = 0;
    volatile int control = volatile_seed & 0xF;
    
    /* Loop with varying patterns */
    for (int i = 0; i < 4; i++) {
        union {
            uint32_t dword;
            struct {
                uint16_t low;
                uint16_t high;
            } words;
        } u;
        
        u.dword = volatile_seed + i;
        
        /* Conditional ZERO_EXTRACT */
        if (control & (1 << i)) {
            result += u.words.high;  /* ZERO_EXTRACT */
        } else {
            result += u.words.low;   /* Another ZERO_EXTRACT */
        }
        
        /* STRICT_LOW_PART in loop */
        char *byte_ptr = (char*)&u.dword;
        byte_ptr[i % 4] = volatile_char + i;
        
        /* Complex MEM access */
        static int static_array[8];
        int idx = (volatile_index + i) & 7;
        static_array[idx] = u.dword;
        
        /* SUBREG access */
        short *short_ptr = (short*)&static_array[idx];
        result += short_ptr[0];
    }
    
    /* Switch statement with different patterns */
    switch (control & 0x3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            int x = volatile_seed;
            result += (x >> 16) & 0xFFFF;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            int y = volatile_seed;
            *(char*)&y = volatile_char;
            result += y;
            break;
        }
        case 2: {
            /* SUBREG + MEM pattern */
            int z[2] = {volatile_seed, volatile_seed + 1};
            short *sp = (short*)z;
            result += sp[volatile_index & 1];
            break;
        }
        default:
            result += volatile_seed;
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Nested expressions that trigger recursive marking */
__attribute__((noinline))
int pattern_nested(void) {
    /* Create a complex nested memory access */
    struct {
        int header;
        union {
            int data[4];
            struct {
                short low[2];
                short high[2];
            } parts;
        } payload;
    } obj;
    
    obj.header = volatile_seed;
    
    /* Nested ZERO_EXTRACT within memory access */
    for (int i = 0; i < 4; i++) {
        obj.payload.data[i] = volatile_seed + i * 0x100;
    }
    
    /* Access causing multiple levels of indirection */
    int *ptr_array[2];
    ptr_array[0] = &obj.payload.data[0];
    ptr_array[1] = &obj.payload.data[2];
    
    /* Complex expression combining multiple patterns */
    int result = 0;
    for (int i = 0; i < 2; i++) {
        /* MEM access with address calculation */
        int *ptr = ptr_array[i];
        
        /* ZERO_EXTRACT from memory */
        int val = *ptr;
        result += (val >> 8) & 0xFF;  /* ZERO_EXTRACT */
        
        /* STRICT_LOW_PART store back */
        char *byte_ptr = (char*)ptr;
        byte_ptr[1] = volatile_char + i;  /* Modify middle byte */
        
        /* SUBREG access */
        short *short_ptr = (short*)ptr;
        result += short_ptr[i];  /* Access as short */
    }
    
    /* Final complex expression */
    result += ((obj.payload.parts.low[0] << 8) | 
               (obj.payload.parts.high[1] & 0xFF));
    
    use_int(result);
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Execute each pattern generator */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_nested();
    
    /* Add some volatile-dependent control flow */
    if (volatile_seed & 1) {
        checksum += pattern_zero_extract();
    }
    
    if (volatile_seed & 2) {
        checksum += pattern_strict_low_part();
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
void use_int(int x) {
    /* Prevent optimization */
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
