/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile inputs to prevent constant folding */
volatile int volatile_seed = 42;
volatile int volatile_index = 3;
volatile short volatile_short = 0xABCD;
volatile char volatile_char = 0x7F;

/* Pattern 1: ZERO_EXTRACT through bitfield unions */
__attribute__((noinline))
int pattern_zero_extract(void) {
    /* Union with bitfields to generate ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low : 8;
            uint32_t middle : 12;
            uint32_t high : 12;
        } bits;
    } u;
    
    u.full = volatile_seed;
    
    /* Multiple extractions to increase chances */
    uint32_t result1 = u.bits.low;
    uint32_t result2 = u.bits.middle;
    uint32_t result3 = u.bits.high;
    
    /* Complex expression with ZERO_EXTRACT */
    uint32_t combined = (u.bits.low << 16) | (u.bits.middle << 4) | u.bits.high;
    
    /* Use results to prevent dead code elimination */
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return combined & 0xFFFF;
}

/* Pattern 2: STRICT_LOW_PART through partial register updates */
__attribute__((noinline))
int pattern_strict_low_part(void) {
    struct {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } data;
    
    data.full = 0;
    
    /* These assignments should generate STRICT_LOW_PART */
    data.parts.low = volatile_short;
    data.parts.high = volatile_short >> 4;
    
    /* Another STRICT_LOW_PART pattern */
    uint32_t temp = volatile_seed;
    uint8_t *byte_ptr = (uint8_t*)&temp;
    byte_ptr[0] = volatile_char;  /* Modify only low byte */
    byte_ptr[1] = volatile_char ^ 0xFF;
    
    use_int(data.full);
    use_int(temp);
    
    return data.parts.low + temp;
}

/* Pattern 3: SUBREG and MEM_P with complex addressing */
__attribute__((noinline))
int pattern_subreg_mem(void) {
    int array[16];
    volatile int idx = volatile_index;
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* SUBREG pattern: access through different-sized types */
    uint32_t *int_ptr = &array[0];
    uint16_t *short_ptr = (uint16_t*)int_ptr;
    uint8_t *byte_ptr = (uint8_t*)int_ptr;
    
    /* Complex MEM_P with address computation */
    int offset = idx * 2 + 1;
    int *mem_ptr = (int*)((char*)array + offset * sizeof(int));
    
    /* Multiple memory accesses with different types */
    uint16_t short_val = short_ptr[idx];
    uint8_t byte_val = byte_ptr[idx * 4];
    int int_val = mem_ptr[-1];  /* Negative offset for complexity */
    
    /* Use pointers to keep computations alive */
    use_short(short_val);
    use_int(byte_val);
    use_int(int_val);
    use_ptr(mem_ptr);
    
    return short_val + byte_val + int_val;
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
int pattern_combined(void) {
    int result = 0;
    volatile int control = volatile_seed & 0x3;
    
    /* Switch with different patterns in each case */
    switch (control) {
        case 0: {
            /* ZERO_EXTRACT in loop */
            union {
                uint64_t full;
                struct {
                    uint32_t low;
                    uint32_t high;
                } words;
            } u64;
            
            u64.full = (uint64_t)volatile_seed << 32 | volatile_seed;
            for (int i = 0; i < 4; i++) {
                uint32_t extracted = (u64.words.low >> (i * 8)) & 0xFF;
                result += extracted;
            }
            break;
        }
        
        case 1: {
            /* STRICT_LOW_PART with pointer arithmetic */
            uint32_t buffer[4];
            uint16_t *half_ptr = (uint16_t*)buffer;
            
            for (int i = 0; i < 8; i++) {
                half_ptr[i] = volatile_short + i;  /* STRICT_LOW_PART */
            }
            
            /* MEM_P with complex address */
            uint32_t *alias_ptr = (uint32_t*)((char*)buffer + 1);
            result = *alias_ptr;  /* Unaligned access */
            break;
        }
        
        case 2: {
            /* Nested SUBREG accesses */
            struct {
                uint64_t data[2];
            } s;
            
            s.data[0] = volatile_seed;
            s.data[1] = volatile_seed * 2;
            
            /* Access as different types */
            uint32_t *as_int = (uint32_t*)&s;
            uint16_t *as_short = (uint16_t*)&s;
            uint8_t *as_byte = (uint8_t*)&s;
            
            result = as_int[1] + as_short[3] + as_byte[7];
            break;
        }
        
        default:
            /* Mixed patterns */
            result = pattern_zero_extract() + 
                    pattern_strict_low_part() +
                    pattern_subreg_mem();
            break;
    }
    
    return result;
}

/* Pattern 5: Deeply nested expressions */
__attribute__((noinline))
int pattern_deep_nesting(void) {
    /* Create complex expression tree */
    uint32_t a = volatile_seed;
    uint32_t b = volatile_seed + 1;
    uint32_t c = volatile_seed + 2;
    
    /* Structure with bitfields for ZERO_EXTRACT */
    struct {
        uint32_t x : 10;
        uint32_t y : 10;
        uint32_t z : 12;
    } bits;
    
    bits.x = a & 0x3FF;
    bits.y = b & 0x3FF;
    bits.z = c & 0xFFF;
    
    /* Array with SUBREG accesses */
    uint32_t arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = i * 100 + volatile_seed;
    }
    
    /* Pointer chain for MEM_P */
    uint32_t *ptr1 = &arr[0];
    uint16_t *ptr2 = (uint16_t*)ptr1;
    uint8_t *ptr3 = (uint8_t*)ptr2;
    
    /* Complex address computation */
    uint32_t idx = (volatile_index * 3) % 8;
    uint32_t *final_ptr = (uint32_t*)(ptr3 + idx * sizeof(uint32_t));
    
    /* Combine everything */
    uint32_t result = bits.x + bits.y + bits.z;
    result += *final_ptr;
    result += ptr2[idx * 2];
    
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Execute all patterns */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_deep_nesting();
    
    /* Use volatile to ensure execution */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", checksum);
    
    return final_result & 0xFF;
}

/* Weak definitions of external functions to allow linking */
void __attribute__((weak)) use_int(int x) { (void)x; }
void __attribute__((weak)) use_short(short x) { (void)x; }
void __attribute__((weak)) use_ptr(void* x) { (void)x; }
void __attribute__((weak)) use_long(long x) { (void)x; }
