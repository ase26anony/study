#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global arrays for memory access patterns */
volatile int arr_int[32];
volatile short arr_short[64];
volatile char arr_char[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
    volatile unsigned int padding;
};

struct nested_bitfield {
    struct {
        unsigned int low : 16;
        unsigned int high : 16;
    } parts;
    volatile int control;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct nested_bitfield bf2;

/* Function to create complex addressing modes */
static int complex_address(int index, int offset) {
    /* Force non-trivial addressing: arr[(index + offset) % size] */
    return (index * 3 + offset * 7) & 31;
}

/* Function to perform bit manipulation that may generate ZERO_EXTRACT */
static unsigned int extract_bits(volatile unsigned int source, 
                                 int start, int length) {
    /* Explicit bit extraction - may generate ZERO_EXTRACT */
    return (source >> start) & ((1u << length) - 1);
}

/* Function with type punning for SUBREG generation */
static short type_punning_int_to_short(volatile int *ptr) {
    /* Cast between different-sized types */
    return *(volatile short *)ptr;
}

static char type_punning_short_to_char(volatile short *ptr) {
    /* Another type punning operation */
    return *(volatile char *)ptr;
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr_int[i] = i * 0x11111111;
    }
    for (i = 0; i < 64; i++) {
        arr_short[i] = i * 0x1111;
    }
    for (i = 0; i < 128; i++) {
        arr_char[i] = i * 0x11;
    }
    
    /* Initialize bitfields */
    bf1.a = 0xA;
    bf1.b = 0xBC;
    bf1.c = 0xDEF;
    bf1.d = 0x12;
    bf1.padding = 0xDEADBEEF;
    
    bf2.parts.low = 0x1234;
    bf2.parts.high = 0x5678;
    bf2.control = 0xCAFEBABE;
    
    /* Use command line arguments to create dynamic values */
    int base_index = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    int loop_count = (argc > 3) ? atoi(argv[3]) % 8 + 4 : 8;
    
    /* Main loop combining all patterns */
    for (i = 0; i < loop_count; i++) {
        volatile int temp;
        
        /* Pattern 1: Memory access with complex addressing (MEM_P) */
        int idx = complex_address(i, base_index);
        temp = arr_int[idx];
        checksum ^= temp;
        
        /* Pattern 2: Bitfield extraction from memory (ZERO_EXTRACT) */
        unsigned int extracted = extract_bits(temp, shift_amount + i, 6);
        checksum += extracted;
        
        /* Pattern 3: Direct bitfield struct member access */
        volatile unsigned int *bf_ptr = &bf1.padding;
        checksum ^= *bf_ptr;
        
        /* Pattern 4: Type punning for SUBREG generation */
        if (i & 1) {
            short sval = type_punning_int_to_short(&temp);
            checksum += sval;
            
            char cval = type_punning_short_to_char((volatile short *)&temp);
            checksum ^= cval;
        }
        
        /* Pattern 5: Combined operation: memory + bit extract + type conversion */
        volatile int combined = arr_short[i * 2] | (arr_short[i * 2 + 1] << 16);
        short low_part = (combined >> 8) & 0xFF;  /* Potential ZERO_EXTRACT */
        checksum += low_part;
        
        /* Pattern 6: Access bitfield through pointer with offset */
        if (i & 2) {
            /* This may generate STRICT_LOW_PART for bitfield assignment */
            struct bitfield_struct *bf_ptr2 = (struct bitfield_struct *)&bf1;
            bf_ptr2->b = (checksum & 0xFF);  /* Bitfield store */
            checksum ^= bf_ptr2->a;
        }
        
        /* Pattern 7: Nested bitfield access */
        unsigned int nested_val;
        if (i & 4) {
            nested_val = bf2.parts.low;  /* Bitfield read */
        } else {
            nested_val = bf2.parts.high; /* Bitfield read */
        }
        checksum ^= nested_val;
        
        /* Pattern 8: Complex expression with multiple memory accesses */
        int idx2 = (i * 7 + base_index) & 63;
        int idx3 = (i * 11 + shift_amount) & 127;
        int complex_val = (arr_short[idx2] << 8) | arr_char[idx3];
        checksum += complex_val & 0xFFFF;  /* Potential STRICT_LOW_PART */
        
        /* Pattern 9: Pointer arithmetic creating complex address */
        volatile int *ptr = &arr_int[0] + ((i * 13) & 31);
        checksum ^= *ptr;
        
        /* Pattern 10: Inline asm to force partial register access */
        asm volatile("" : "+r"(checksum) : : "memory");
    }
    
    /* Additional complex statement combining multiple patterns */
    /* Memory access + bit extraction + type conversion in one expression */
    volatile int final_val = 
        (*(volatile short*)((char*)&arr_int[0] + (base_index * 4)) >> shift_amount) & 0x1F;
    checksum ^= final_val;
    
    /* Access bitfield through volatile pointer */
    volatile unsigned int *volatile_bf_ptr = (volatile unsigned int*)&bf1;
    checksum += *volatile_bf_ptr;
    
    printf("Checksum: %u\n", checksum);
    return checksum & 0xFF;
}
