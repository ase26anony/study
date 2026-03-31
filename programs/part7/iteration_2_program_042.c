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

/* Pattern 1: Generate ZERO_EXTRACT operations */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Method 1: Using bitfields in union */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 8;
            uint32_t high: 16;
        } bits;
    } u;
    
    u.full = volatile_seed;
    int result1 = u.bits.high;  /* Should generate ZERO_EXTRACT */
    
    /* Method 2: Manual masking and shifting */
    uint32_t val = volatile_seed;
    int result2 = (val >> 16) & 0xFFFF;  /* Alternative ZERO_EXTRACT pattern */
    
    /* Method 3: Extract multiple fields */
    int result3 = (val >> 8) & 0xFF;
    int result4 = (val >> 24) & 0xFF;
    
    /* Combine results to prevent dead code elimination */
    return result1 + result2 + result3 + result4;
}

/* Pattern 2: Generate STRICT_LOW_PART operations */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    /* Method 1: Structure with small member */
    struct {
        char low_byte;
        int rest;
    } s1;
    
    int temp = volatile_seed;
    s1.low_byte = temp & 0xFF;  /* Should generate STRICT_LOW_PART */
    
    /* Method 2: Pointer to low part */
    int value = volatile_seed;
    char *low_ptr = (char*)&value;
    *low_ptr = volatile_mask & 0x7F;  /* Modify only low byte */
    
    /* Method 3: Union type punning */
    union {
        int32_t full;
        int8_t parts[4];
    } u2;
    
    u2.full = volatile_seed;
    u2.parts[0] = volatile_mask;  /* Modify low part only */
    
    /* Use results to keep them alive */
    use_int(s1.low_byte);
    use_int(*low_ptr);
    use_int(u2.parts[0]);
    
    return s1.low_byte + *low_ptr + u2.parts[0];
}

/* Pattern 3: Generate SUBREG operations */
__attribute__((noinline))
static int pattern_subreg(void) {
    /* Method 1: Array access with type conversion */
    int32_t array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Access as different types - should generate SUBREG */
    int16_t *short_ptr = (int16_t*)array;
    int16_t short_val = short_ptr[volatile_index];  /* SUBREG of MEM */
    
    /* Method 2: Direct type punning */
    int64_t big_val = (int64_t)volatile_seed * 2;
    int32_t *half_ptr = (int32_t*)&big_val;
    int32_t half_val = half_ptr[volatile_index & 1];  /* SUBREG access */
    
    /* Method 3: Structure field access */
    struct mixed {
        int32_t a;
        int16_t b;
        int8_t c;
    } m;
    
    m.a = volatile_seed;
    m.b = volatile_seed & 0xFFFF;
    m.c = volatile_seed & 0xFF;
    
    int16_t b_val = m.b;  /* SUBREG access to structure field */
    
    use_short(short_val);
    use_int(half_val);
    use_short(b_val);
    
    return short_val + half_val + b_val;
}

/* Pattern 4: Generate complex MEM_P patterns */
__attribute__((noinline))
static int pattern_mem_complex(void) {
    /* Complex addressing modes */
    int buffer[16];
    volatile int offset = volatile_index;
    
    /* Initialize buffer */
    for (int i = 0; i < 16; i++) {
        buffer[i] = volatile_seed + i * 100;
    }
    
    /* Method 1: Pointer arithmetic with different types */
    char *char_base = (char*)buffer;
    int *int_ptr = (int*)(char_base + offset * sizeof(int));
    int val1 = *int_ptr;  /* MEM with complex address */
    
    /* Method 2: Nested pointer dereference */
    int **ptr_ptr = &int_ptr;
    int val2 = **ptr_ptr;  /* MEM of MEM */
    
    /* Method 3: Array indexing with computation */
    int index = (offset * 3 + 7) & 0xF;
    int val3 = buffer[index];  /* MEM with computed index */
    
    /* Method 4: Structure pointer chain */
    struct node {
        int data;
        struct node *next;
    } nodes[4];
    
    for (int i = 0; i < 4; i++) {
        nodes[i].data = volatile_seed + i * 10;
        nodes[i].next = &nodes[(i + 1) & 3];
    }
    
    struct node *current = &nodes[offset & 3];
    int val4 = current->next->next->data;  /* Complex MEM chain */
    
    use_int(val1);
    use_int(val2);
    use_int(val3);
    use_int(val4);
    
    return val1 + val2 + val3 + val4;
}

/* Pattern 5: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    int result = 0;
    volatile int control = volatile_seed;
    
    /* Loop with combined operations */
    for (int i = 0; i < 4; i++) {
        /* ZERO_EXTRACT in loop */
        uint32_t val = volatile_seed + i;
        int extracted = (val >> (i * 4)) & 0xF;
        
        /* STRICT_LOW_PART assignment */
        struct {
            char low;
            char high;
        } parts;
        parts.low = extracted & 0x7;
        
        /* SUBREG memory access */
        int array[2] = {volatile_seed, volatile_seed + 1};
        short *short_view = (short*)array;
        short_view[i & 1] = parts.low;
        
        /* Complex MEM access based on control flow */
        if (control & (1 << i)) {
            int *ptr = &array[(i + 1) & 1];
            result += *ptr;  /* MEM_P with potential SUBREG */
        } else {
            result += extracted;
        }
    }
    
    /* Switch statement with different patterns */
    switch (control & 0x3) {
        case 0: {
            /* ZERO_EXTRACT dominant */
            union {
                uint32_t full;
                uint16_t halves[2];
            } u;
            u.full = volatile_seed;
            result += u.halves[0];
            break;
        }
        case 1: {
            /* STRICT_LOW_PART dominant */
            int temp = volatile_seed;
            char *byte_ptr = (char*)&temp;
            byte_ptr[1] = volatile_mask;
            result += temp;
            break;
        }
        case 2: {
            /* SUBREG dominant */
            long long big = (long long)volatile_seed * volatile_seed;
            int *half = (int*)&big;
            result += half[control & 1];
            break;
        }
        case 3: {
            /* Complex MEM dominant */
            int *indirect = &volatile_index;
            result += *indirect;
            break;
        }
    }
    
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute each pattern */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg();
    checksum += pattern_mem_complex();
    checksum += pattern_combined();
    
    /* Add some volatile-dependent computation */
    checksum += volatile_seed & 0xFF;
    checksum += volatile_index * 2;
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use results to prevent optimization */
    use_int(checksum);
    
    return checksum & 0xFF;
}

/* Dummy external function definitions to satisfy linker */
/* These would normally be in a separate file */
void __attribute__((weak)) use_int(int x) { (void)x; }
void __attribute__((weak)) use_short(short x) { (void)x; }
void __attribute__((weak)) use_ptr(void* x) { (void)x; }
void __attribute__((weak)) use_long(long x) { (void)x; }
