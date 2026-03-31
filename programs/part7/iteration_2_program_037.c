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

/* Pattern 1: Generate ZERO_EXTRACT RTL */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    volatile int input = seed ^ 0x55AA55AA;
    
    /* Method 1: Union with bitfields */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t middle: 12;
            uint32_t high: 12;
        } bits;
    } u;
    
    u.full = input;
    int result1 = u.bits.middle;  /* Should generate ZERO_EXTRACT */
    
    /* Method 2: Manual masking and shifting */
    int temp = input;
    int result2 = (temp >> 4) & 0xFFF;  /* Another potential ZERO_EXTRACT */
    
    /* Method 3: Nested extractions */
    union {
        uint64_t dword;
        struct {
            uint32_t first;
            uint32_t second;
        } words;
    } u2;
    
    u2.dword = (uint64_t)input * 0x100000001ULL;
    int result3 = (u2.words.first >> 8) & 0xFFFF;
    
    /* Use results to keep them alive */
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return result1 + result2 + result3;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    volatile int input = seed ^ 0xAA55AA55;
    
    /* Method 1: Structure with small members */
    struct {
        char low_byte;
        short low_word;
        int full;
    } s;
    
    s.full = input;
    s.low_byte = (input & 0xFF) ^ 0x55;  /* Should generate STRICT_LOW_PART */
    s.low_word = (input & 0xFFFF) ^ 0xAAAA;
    
    /* Method 2: Pointer to low part */
    int temp = input;
    char *low_ptr = (char*)&temp;
    low_ptr[0] = (temp & 0xFF) + 1;  /* Modify only low byte */
    
    /* Method 3: Union type punning */
    union {
        int i;
        struct {
            char b0;
            char b1;
            char b2;
            char b3;
        } bytes;
    } u;
    
    u.i = input;
    u.bytes.b1 = (input >> 8) & 0x7F;  /* Modify specific byte */
    
    /* Use results */
    use_char(s.low_byte);
    use_short(s.low_word);
    use_int(temp);
    use_int(u.i);
    
    return s.low_byte + s.low_word + temp + u.i;
}

/* Pattern 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    volatile int index = (seed >> 4) & 0x7;
    volatile int value = seed ^ 0x33333333;
    
    /* Array with different access patterns */
    int array[16] = {0};
    
    /* Complex memory addressing with SUBREG */
    short *short_ptr = (short*)((char*)array + index * 2);
    *short_ptr = value & 0xFFFF;  /* MEM with address computation */
    
    /* Type punning through different pointer types */
    char *char_base = (char*)array;
    int *int_ptr = (int*)(char_base + index * 4 + 1);
    *int_ptr = value;  /* Unaligned access - may generate interesting MEM */
    
    /* Nested SUBREG through pointer arithmetic */
    struct {
        int a;
        int b;
        short c;
        char d;
    } compound[4];
    
    volatile int idx2 = index & 0x3;
    compound[idx2].c = value & 0x7FFF;  /* SUBREG access to structure member */
    
    /* Pointer chasing with different types */
    void *ptr = array;
    int *alias1 = (int*)((char*)ptr + 8);
    short *alias2 = (short*)((char*)ptr + 10);
    
    *alias1 = value;
    *alias2 = (value >> 16) & 0xFFFF;
    
    /* Use pointers to keep computations */
    use_ptr(short_ptr);
    use_ptr(int_ptr);
    use_ptr(&compound[idx2].c);
    
    return array[0] + array[1] + compound[0].c + *alias2;
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int selector = seed & 0xF;
    volatile int data = seed ^ 0x0F0F0F0F;
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
            u.val = data;
            u.parts.low = (data >> 8) & 0xFF;  /* STRICT_LOW_PART */
            result = (u.val >> 4) & 0xFFF;     /* ZERO_EXTRACT */
            break;
        }
        
        case 1: {
            /* MEM_P with complex address + SUBREG */
            int buffer[8];
            short *p = (short*)((char*)buffer + (selector * 2));
            *p = data & 0x7FFF;
            result = *p + buffer[0];
            break;
        }
        
        case 2: {
            /* Nested patterns in loop */
            for (int i = 0; i < 4; i++) {
                struct {
                    int x;
                    char c;
                } s;
                s.x = data + i;
                s.c = (data >> (i * 2)) & 0xFF;  /* STRICT_LOW_PART in loop */
                result += s.c;
            }
            break;
        }
        
        default: {
            /* Mixed patterns with conditionals */
            if (selector & 1) {
                /* ZERO_EXTRACT path */
                result = (data >> (selector & 0x7)) & ((1 << (selector + 1)) - 1);
            } else {
                /* MEM_P + SUBREG path */
                int temp[4];
                char *base = (char*)temp;
                int offset = (selector * 3) & 0x7;
                *(int*)(base + offset) = data;
                result = temp[0];
            }
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Recursive-like pattern generation */
__attribute__((noinline))
static int pattern_recursive_like(void) {
    volatile int depth = (seed >> 8) & 0x3;
    int result = seed;
    
    /* Generate patterns at different "depths" */
    for (int i = 0; i <= depth; i++) {
        /* Alternate between patterns */
        if (i & 1) {
            /* ZERO_EXTRACT pattern */
            union {
                struct {
                    int a: 10;
                    int b: 10;
                    int c: 12;
                } fields;
                int whole;
            } u;
            u.whole = result;
            result = u.fields.b + (u.fields.c << 10);
        } else {
            /* STRICT_LOW_PART + MEM pattern */
            struct wrapper {
                int data;
                short partial;
            } w;
            w.data = result;
            w.partial = result & 0xFFFF;  /* STRICT_LOW_PART */
            
            /* Access through pointer with offset */
            char *mem = (char*)&w;
            int *alias = (int*)(mem + 2);  /* Potential unaligned access */
            result = *alias + w.partial;
        }
    }
    
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute each pattern generator */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_recursive_like();
    
    /* Add some variation based on seed */
    checksum += (seed & 0xFF);
    
    printf("Final checksum: %d (0x%08X)\n", checksum, checksum);
    
    return checksum & 0xFF;
}

/* Dummy external function definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_char(char x) { (void)x; }
void use_ptr(void* x) { (void)x; }
