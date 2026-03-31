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
    int result = u.bits.high;          /* Should generate ZERO_EXTRACT */
    result += (u.full >> 16) & 0xFFFF; /* Alternative extraction */
    
    /* Complex extraction with control flow */
    if (input & 1) {
        result += u.bits.mid;
    }
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    struct {
        char low;
        char mid;
        short high;
    } s;
    
    volatile int temp = seed + 2;
    
    /* Multiple low-part assignments */
    s.low = temp & 0xFF;               /* Potential STRICT_LOW_PART */
    s.mid = (temp >> 8) & 0xFF;
    
    /* Through pointer with type punning */
    char *ptr = (char*)&s.high;
    *ptr = temp & 0xFF;                /* Another low-part store */
    
    /* In loop to create different contexts */
    for (int i = 0; i < 3; i++) {
        s.low = (temp + i) & 0xFF;
    }
    
    use_short(s.high);
    return s.low + s.mid;
}

/* Pattern 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    int array[16];
    volatile int index = seed % 8;
    volatile short value = seed & 0xFFFF;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 100;
    }
    
    /* Complex memory access patterns */
    
    /* 1. SUBREG through pointer casting */
    short *short_ptr = (short*)((char*)array + index * sizeof(int));
    *short_ptr = value;                /* Generates SUBREG + MEM */
    
    /* 2. Different type access */
    char *char_ptr = (char*)&array[4];
    char_ptr[index] = value & 0xFF;
    
    /* 3. Nested addressing */
    int *int_ptr = &array[index];
    short *nested_ptr = (short*)int_ptr;
    nested_ptr[1] = value >> 8;
    
    /* 4. Pointer arithmetic with different types */
    long *long_ptr = (long*)array;
    use_ptr(long_ptr + index);
    
    return array[0] + array[4];
}

/* Pattern 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    union {
        uint32_t val;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } data;
    
    volatile uint32_t input = seed + 4;
    data.val = input;
    
    int result = 0;
    
    /* Switch with different extraction patterns */
    switch (input & 0x3) {
        case 0:
            /* ZERO_EXTRACT pattern */
            result = data.parts.high;
            break;
        case 1:
            /* STRICT_LOW_PART pattern */
            {
                struct { char a; char b; } s;
                s.a = input & 0xFF;
                s.b = (input >> 8) & 0xFF;
                result = s.a + s.b;
            }
            break;
        case 2:
            /* SUBREG + MEM pattern */
            {
                int buffer[4];
                short *ptr = (short*)buffer;
                ptr[1] = input & 0xFFFF;
                result = buffer[0];
            }
            break;
        default:
            /* Mixed pattern */
            result = (data.val >> 8) & 0xFF;  /* ZERO_EXTRACT */
            {
                char *cptr = (char*)&result;
                cptr[0] = input & 0xFF;       /* STRICT_LOW_PART */
            }
            break;
    }
    
    /* Loop with memory access */
    int temp_array[8];
    for (int i = 0; i < 8; i++) {
        /* Complex addressing */
        short *elem = (short*)((char*)temp_array + i * sizeof(int));
        *elem = (result + i) & 0xFFFF;
    }
    
    use_int(result + temp_array[0]);
    return result;
}

/* Pattern 5: Deeply nested patterns */
__attribute__((noinline))
static int pattern_nested(void) {
    /* Structure with nested bitfields */
    struct {
        union {
            struct {
                unsigned int a : 4;
                unsigned int b : 4;
                unsigned int c : 8;
                unsigned int d : 16;
            } bits;
            uint32_t full;
        } inner;
        short array[4];
    } complex;
    
    volatile uint32_t v = seed + 5;
    complex.inner.full = v;
    
    /* Multiple extractions */
    int r1 = complex.inner.bits.c;     /* ZERO_EXTRACT */
    int r2 = complex.inner.bits.d;
    
    /* Memory access with pointer arithmetic */
    char *base = (char*)complex.array;
    for (int i = 0; i < 4; i++) {
        /* Create SUBREG patterns */
        short *ptr = (short*)(base + i * sizeof(short));
        *ptr = (r1 + i * r2) & 0xFFFF;
        
        /* Conditional low-part assignment */
        if (i & 1) {
            char *cptr = (char*)ptr;
            cptr[0] = v & 0xFF;        /* STRICT_LOW_PART */
        }
    }
    
    use_long(complex.inner.full);
    return r1 + r2 + complex.array[0];
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute all patterns */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_nested();
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", final_result);
    printf("Pattern generation complete.\n");
    
    return final_result & 0xFF;
}

/* External function definitions (weak linkage) */
void __attribute__((weak)) use_int(int x) { (void)x; }
void __attribute__((weak)) use_short(short x) { (void)x; }
void __attribute__((weak)) use_ptr(void* x) { (void)x; }
void __attribute__((weak)) use_long(long x) { (void)x; }
