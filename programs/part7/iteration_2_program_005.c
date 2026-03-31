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
static volatile int volatile_mask = 0xFF;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Union with bitfields to encourage ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    /* Use volatile to prevent constant folding */
    u.full = volatile_seed;
    
    /* Multiple extraction patterns */
    int result = 0;
    
    /* This should generate ZERO_EXTRACT for bitfield access */
    result += u.bits.mid;           /* Extract middle 8 bits */
    result += (u.full >> 4) & 0xF;  /* Manual shift+mask */
    
    /* Nested extraction */
    union {
        uint32_t dword;
        struct {
            uint16_t word1;
            uint16_t word2;
        } words;
    } v;
    v.dword = volatile_seed ^ 0x87654321;
    result += v.words.word1;
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    int result = 0;
    
    /* Structure with small member to encourage STRICT_LOW_PART */
    struct {
        char low_byte;
        char padding[3];
        int full_word;
    } s;
    
    /* Initialize with volatile */
    int temp = volatile_seed;
    
    /* These assignments should generate STRICT_LOW_PART */
    s.low_byte = temp & 0xFF;          /* Only modify low byte */
    s.full_word = temp;                /* Modify full word */
    
    /* Pointer-based low-part modification */
    short *short_ptr = (short*)&s.full_word;
    *short_ptr = (short)(temp >> 8);   /* Modify low 16 bits only */
    
    /* Union-based approach */
    union {
        int full;
        struct {
            unsigned char b0, b1, b2, b3;
        } bytes;
    } u;
    u.full = 0;
    u.bytes.b1 = (temp >> 8) & 0xFF;   /* Modify only byte 1 */
    
    result = s.low_byte + *short_ptr + u.bytes.b1;
    return result;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    int result = 0;
    
    /* Array for memory access patterns */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Complex pointer arithmetic for MEM with address computation */
    volatile int idx = volatile_index;
    
    /* Different type accesses to same memory - should generate SUBREG */
    int *int_ptr = &array[idx];
    short *short_ptr = (short*)int_ptr;
    char *char_ptr = (char*)int_ptr;
    
    /* Access through different type pointers */
    result += *int_ptr;                /* Full word access */
    result += short_ptr[1];            /* Short access with offset */
    result += char_ptr[3];             /* Byte access */
    
    /* Pointer arithmetic with different types */
    long *long_ptr = (long*)array;
    result += (int)long_ptr[idx % 8];  /* Long access */
    
    /* Nested structure with array */
    struct {
        int header;
        short data[10];
        int footer;
    } buffer;
    
    buffer.header = volatile_seed;
    for (int i = 0; i < 10; i++) {
        buffer.data[i] = (short)(volatile_seed + i);
    }
    buffer.footer = volatile_seed ^ 0xFFFFFFFF;
    
    /* Access through computed pointer */
    short *data_ptr = &buffer.data[idx % 10];
    result += *data_ptr;
    
    return result;
}

/* Function 4: Combined pattern - all three in one */
__attribute__((noinline))
static int pattern_combined(void) {
    /* Combined union for ZERO_EXTRACT */
    union {
        uint32_t value;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
        unsigned char bytes[4];
    } data;
    
    data.value = volatile_seed;
    
    /* ZERO_EXTRACT pattern */
    uint16_t extracted = data.parts.high;  /* Should be ZERO_EXTRACT */
    
    /* Structure for STRICT_LOW_PART */
    struct {
        int base;
        short extension;
    } container;
    
    /* STRICT_LOW_PART pattern */
    container.base = 0;
    container.extension = (short)extracted;  /* Modify only low part */
    
    /* Memory array for SUBREG/MEM patterns */
    int memory[8];
    for (int i = 0; i < 8; i++) {
        memory[i] = data.value + i * 0x11111111;
    }
    
    /* Complex addressing mode */
    volatile int offset = volatile_index;
    int *mem_ptr = memory + (offset & 3);
    
    /* Access through different type */
    short *short_mem = (short*)mem_ptr;
    int mem_result = short_mem[1];  /* SUBREG + MEM */
    
    return container.extension + mem_result + data.bytes[2];
}

/* Function 5: Control flow variation with patterns */
__attribute__((noinline))
static int pattern_with_control_flow(void) {
    int result = 0;
    volatile int control = volatile_seed;
    
    /* Loop with pattern inside */
    for (int i = 0; i < 4; i++) {
        /* ZERO_EXTRACT in loop */
        union {
            int val;
            struct {
                unsigned int a: 10;
                unsigned int b: 10;
                unsigned int c: 12;
            } fields;
        } u;
        u.val = control + i;
        result += u.fields.b;  /* ZERO_EXTRACT */
        
        /* Conditional STRICT_LOW_PART */
        if (i & 1) {
            struct {
                short low;
                short high;
            } s;
            s.low = (short)(u.val & 0xFFFF);
            s.high = (short)(u.val >> 16);
            result += s.low;  /* STRICT_LOW_PART in store */
        }
    }
    
    /* Switch with different MEM patterns */
    switch (control & 3) {
        case 0: {
            int arr[4];
            short *sp = (short*)arr;
            sp[1] = result & 0xFFFF;  /* SUBREG + MEM */
            result = arr[0];
            break;
        }
        case 1: {
            char buffer[16];
            int *ip = (int*)(buffer + 4);
            *ip = result;  /* MEM with offset */
            result = buffer[4];
            break;
        }
        default: {
            long *lp = (long*)&result;
            result = (int)(*lp & 0xFFFFFFFF);  /* SUBREG */
            break;
        }
    }
    
    return result;
}

/* Main function that calls all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Call each pattern function */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_with_control_flow();
    
    /* Use external function to prevent dead code elimination */
    use_int(checksum);
    
    printf("Checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum & 0xFF;
}

/* Dummy definitions for external functions (in same file for testing) */
void use_int(int x) { printf("use_int: %d\n", x); }
void use_short(short x) { printf("use_short: %d\n", (int)x); }
void use_ptr(void* x) { printf("use_ptr: %p\n", x); }
void use_long(long x) { printf("use_long: %ld\n", x); }
