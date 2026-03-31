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

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline)) 
static int test_zero_extract(void) {
    /* Bitfield extraction using union */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 12;
            uint32_t high: 12;
        } bits;
    } u;
    
    u.full = volatile_seed;
    
    /* Multiple extractions to increase chances */
    int result1 = u.bits.mid;           /* Should generate ZERO_EXTRACT */
    int result2 = u.bits.high << 4;     /* Shifted extraction */
    
    /* Another extraction pattern using masking */
    uint32_t mask = 0x0FFF0000;
    int result3 = (volatile_seed & mask) >> 16;
    
    /* Combine results */
    int final = result1 + result2 + result3;
    use_int(final);
    return final;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline)) 
static int test_strict_low_part(void) {
    struct {
        unsigned char low_byte;
        unsigned char mid_byte;
        unsigned short word;
        int full;
    } data;
    
    /* Initialize with volatile values */
    data.full = volatile_seed;
    
    /* Modify only low parts - should generate STRICT_LOW_PART */
    data.low_byte = volatile_char & 0xFF;
    data.mid_byte = (volatile_seed >> 8) & 0xFF;
    
    /* Modify low word */
    data.word = volatile_short;
    
    /* Access through pointer to low part */
    unsigned char *ptr = &data.low_byte;
    *ptr = volatile_char + 1;
    
    use_short(data.word);
    return data.full;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline)) 
static int test_subreg_mem(void) {
    /* Array with type punning */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Complex pointer arithmetic - should generate SUBREG and MEM */
    volatile int idx = volatile_index;
    
    /* Access through different type pointers */
    short *short_ptr = (short*)((char*)array + idx * sizeof(int));
    *short_ptr = volatile_short;  /* MEM with SUBREG */
    
    /* Another SUBREG pattern */
    long long big_val = (long long)volatile_seed * 2;
    int *int_ptr = (int*)&big_val;
    int low_part = int_ptr[0];  /* Access low 32 bits */
    int high_part = int_ptr[1]; /* Access high 32 bits */
    
    /* Complex addressing mode */
    int (*array_ptr)[4] = (int(*)[4])array;
    int val = array_ptr[idx][idx % 4];
    
    use_int(low_part + high_part + val);
    return *short_ptr + val;
}

/* Function 4: Combined patterns */
__attribute__((noinline)) 
static int test_combined(void) {
    /* Structure with bitfields and regular members */
    struct combined {
        unsigned int field1: 10;
        unsigned int field2: 6;
        unsigned int field3: 16;
        unsigned char bytes[4];
        int full_word;
    } cmb;
    
    cmb.full_word = volatile_seed;
    
    /* ZERO_EXTRACT from bitfield */
    int extracted = cmb.field2;
    
    /* STRICT_LOW_PART assignment */
    cmb.bytes[0] = extracted & 0xFF;
    
    /* MEM with SUBREG through pointer */
    unsigned char *byte_ptr = (unsigned char*)&cmb.full_word;
    byte_ptr[volatile_index % 4] = volatile_char;
    
    /* Complex memory access */
    int *aliased = (int*)cmb.bytes;
    *aliased = (*aliased & 0xFFFF0000) | (volatile_short & 0xFFFF);
    
    use_int(cmb.full_word);
    return cmb.full_word + extracted;
}

/* Function 5: Control flow variations with patterns */
__attribute__((noinline)) 
static int test_control_flow(void) {
    int result = 0;
    volatile int limit = 5;
    
    /* Loop with pattern inside */
    for (volatile int i = 0; i < limit; i++) {
        union {
            uint32_t val;
            struct {
                uint32_t a: 5;
                uint32_t b: 11;
                uint32_t c: 16;
            } parts;
        } u;
        
        u.val = volatile_seed + i;
        
        /* Conditional ZERO_EXTRACT */
        if (i & 1) {
            result += u.parts.b;  /* ZERO_EXTRACT */
        } else {
            result += u.parts.c;  /* Another ZERO_EXTRACT */
        }
        
        /* Switch with different patterns */
        switch (i % 3) {
            case 0: {
                /* STRICT_LOW_PART pattern */
                struct { short low; } s;
                s.low = result & 0xFFFF;
                result = s.low;
                break;
            }
            case 1: {
                /* SUBREG pattern */
                int arr[2] = {result, volatile_seed};
                short *sp = (short*)arr;
                result = sp[1];  /* SUBREG access */
                break;
            }
            case 2: {
                /* MEM pattern with complex address */
                int *ptr = &result + i;
                result = *ptr;
                break;
            }
        }
    }
    
    use_int(result);
    return result;
}

/* Main function that calls all test patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all pattern generators */
    checksum ^= test_zero_extract();
    checksum ^= test_strict_low_part();
    checksum ^= test_subreg_mem();
    checksum ^= test_combined();
    checksum ^= test_control_flow();
    
    /* Use volatile to ensure all computations are kept */
    volatile int final_result = checksum;
    
    printf("Final checksum: 0x%08X\n", final_result);
    
    return final_result & 0xFF;
}

/* External function definitions (should be in separate file for best results) */
void use_int(int x) {
    /* Empty - just to prevent optimization */
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
