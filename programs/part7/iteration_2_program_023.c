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
            uint32_t middle: 8;
            uint32_t high: 16;
        } bits;
    } u;
    
    volatile uint32_t input = seed + 1;
    u.full = input;
    
    /* Multiple extractions to increase chances */
    int result = 0;
    result += u.bits.low;           /* Should generate ZERO_EXTRACT */
    result += u.bits.middle << 8;   /* Another extraction */
    result += u.bits.high << 16;    /* And another */
    
    /* Manual masking that might also generate ZERO_EXTRACT */
    uint32_t mask = (input >> 4) & 0xF;
    result += mask;
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART operations */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    struct {
        unsigned char low;
        unsigned char mid;
        unsigned short high;
    } s;
    
    volatile int temp = seed + 2;
    
    /* Assignments to partial registers */
    s.low = temp & 0xFF;            /* Potential STRICT_LOW_PART */
    s.mid = (temp >> 8) & 0xFF;     /* Another partial assignment */
    
    /* Through pointer to force memory access */
    unsigned char *ptr = &s.high;
    *ptr = (temp >> 16) & 0xFF;     /* Another partial store */
    
    /* Union-based partial assignment */
    union {
        uint32_t full;
        struct {
            uint16_t low16;
            uint16_t high16;
        } parts;
    } u;
    
    u.full = temp;
    u.parts.low16 = (temp + 1) & 0xFFFF;  /* STRICT_LOW_PART candidate */
    
    use_int(s.low + s.mid + u.parts.low16);
    return s.low + s.mid + u.parts.low16;
}

/* Pattern 3: Generate SUBREG and complex MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    int array[16];
    volatile int index = seed % 8;
    volatile int value = seed + 3;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 100;
    }
    
    /* Type punning through different pointer types */
    char *char_ptr = (char *)array;
    short *short_ptr = (short *)(char_ptr + index * sizeof(int));
    
    /* SUBREG through pointer arithmetic */
    *short_ptr = value & 0xFFFF;    /* SUBREG store */
    
    /* Complex addressing mode */
    int *int_ptr = (int *)((char *)array + index * 2 + 1);
    *int_ptr = value;               /* MEM with complex address */
    
    /* Nested SUBREG through union */
    union {
        uint64_t dword;
        struct {
            uint32_t low;
            uint32_t high;
        } words;
    } u;
    
    u.dword = (uint64_t)value * 1000;
    array[0] = u.words.low;         /* Potential SUBREG extraction */
    array[1] = u.words.high;
    
    use_ptr(array);
    return array[0] + array[1] + *short_ptr;
}

/* Pattern 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int selector = seed % 4;
    int result = 0;
    
    /* Switch with different pattern combinations */
    switch (selector) {
        case 0: {
            /* ZERO_EXTRACT + STRICT_LOW_PART */
            union {
                uint32_t val;
                struct {
                    uint16_t low;
                    uint16_t high;
                } parts;
            } u;
            
            u.val = seed + 100;
            u.parts.low = (seed + 200) & 0xFFFF;  /* STRICT_LOW_PART */
            result = u.parts.high >> 8;           /* ZERO_EXTRACT */
            break;
        }
            
        case 1: {
            /* MEM_P with SUBREG addressing */
            int buffer[8];
            volatile int idx = (seed >> 2) & 3;
            
            /* Complex pointer chain */
            char *base = (char *)buffer;
            int *ptr = (int *)(base + idx * sizeof(short) + 1);
            *ptr = seed + 300;
            
            /* Access through different type */
            short *sptr = (short *)ptr;
            result = *sptr;  /* SUBREG load */
            break;
        }
            
        case 2: {
            /* Nested extractions */
            uint32_t val = seed + 400;
            /* Multiple ZERO_EXTRACT operations */
            result = (val & 0xFF) + 
                    ((val >> 8) & 0xFF) + 
                    ((val >> 16) & 0xF) + 
                    ((val >> 20) & 0xF);
            break;
        }
            
        default: {
            /* Mixed operations in loop */
            struct {
                unsigned char bytes[4];
                unsigned short words[2];
            } data;
            
            for (int i = 0; i < 4; i++) {
                data.bytes[i] = (seed >> (i * 8)) & 0xFF;  /* STRICT_LOW_PART */
            }
            
            /* Convert bytes to words with SUBREG */
            data.words[0] = data.bytes[0] | (data.bytes[1] << 8);
            data.words[1] = data.bytes[2] | (data.bytes[3] << 8);
            
            result = data.words[0] + data.words[1];
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Complex memory addressing with arithmetic */
__attribute__((noinline))
static int pattern_complex_address(void) {
    struct {
        int data[10];
        volatile int offset;
    } ctx;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        ctx.data[i] = i * 1000;
    }
    ctx.offset = (seed % 7) + 1;
    
    /* Complex address calculation that might generate MEM with arithmetic */
    int *ptr1 = &ctx.data[ctx.offset];
    int *ptr2 = &ctx.data[ctx.offset * 2 % 10];
    
    /* Operation that uses both pointers */
    *ptr1 = *ptr2 + seed;
    
    /* SUBREG access to part of the data */
    short *half_ptr = (short *)ptr1;
    int temp = *half_ptr;  /* SUBREG load */
    
    /* More complex: pointer to middle of array element */
    char *byte_ptr = (char *)&ctx.data[5];
    byte_ptr += ctx.offset;
    *byte_ptr = temp & 0xFF;  /* STRICT_LOW_PART store */
    
    use_long((long)ptr1 + (long)ptr2);
    return *ptr1 + temp + *byte_ptr;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Execute all patterns */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_complex_address();
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", final_result);
    printf("Pattern generation complete.\n");
    
    return final_result & 0xFF;
}

/* External function definitions (in separate file normally) */
#ifdef DEFINE_EXTERNAL_FUNCTIONS
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
#endif
