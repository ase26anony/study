/* test_resource_coverage.c */

#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct BitfieldStruct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int padding : 16;
};

volatile struct BitfieldStruct global_bitfield;

/* Function to set bitfield via pointer - ensures memory destination */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, unsigned int val) {
    /* Multiple bitfield assignments to increase pattern visibility */
    ptr->field1 = val & 0x7;
    ptr->field2 = (val >> 3) & 0x1F;
    ptr->field3 = (val >> 8) & 0xFF;
}

/* Function with volatile bitfield - forces memory access */
void set_volatile_bitfield(void) {
    volatile struct {
        unsigned int status : 2;
        unsigned int control : 4;
    } device_reg;
    
    device_reg.status = 1;
    device_reg.control = 0xA;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
}

/* Function using inline assembly for STRICT_LOW_PART pattern */
void partial_register_operations(void) {
    uint32_t value = 0x12345678;
    uint8_t low_byte;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    asm volatile(
        "movb %1, %0\n\t"
        : "=Q" (low_byte)  /* Q constraint = byte-addressable register */
        : "r" ((uint8_t)value)
        : /* no clobbers */
    );
    
    /* Store to memory to ensure MEM reference */
    volatile uint8_t *mem_loc = (volatile uint8_t *)&global_bitfield;
    *mem_loc = low_byte;
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_bitfield_operations(int iterations, struct BitfieldStruct *arr) {
    volatile int counter = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional bitfield assignment based on external input */
        if (i & 1) {
            arr[i].field1 = counter++ & 0x7;
        } else {
            arr[i].field2 = (counter++ >> 1) & 0x1F;
        }
        
        /* Inline assembly that reads/writes memory */
        asm volatile(
            "movl %1, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            "movb %%al, %0\n\t"
            : "=m" (arr[i].field1)
            : "r" (i)
            : "eax", "memory"
        );
        
        /* Atomic operation on bitfield - may generate ZERO_EXTRACT */
        __sync_fetch_and_or(&arr[i].field3, 0x1);
    }
}

/* Function with unpredictable control flow to prevent optimization */
void unpredictable_bitfield_set(struct BitfieldStruct *ptr, int condition) {
    /* Use condition to make assignments non-eliminable */
    if (condition & 0x1) {
        ptr->field1 = 2;
    }
    if (condition & 0x2) {
        ptr->field2 = 0x1F;
    }
    if (condition & 0x4) {
        ptr->field3 = 0xFF;
    }
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    struct BitfieldStruct local_bitfield;
    struct BitfieldStruct *dynamic_bitfield;
    
    /* Use argc to make control flow unpredictable */
    int condition = argc;
    
    /* Initialize */
    local_bitfield.field1 = 0;
    local_bitfield.field2 = 0;
    local_bitfield.field3 = 0;
    
    /* Allocate on heap to ensure memory reference */
    dynamic_bitfield = (struct BitfieldStruct *)malloc(sizeof(struct BitfieldStruct) * 10);
    if (!dynamic_bitfield) return 1;
    
    /* Exercise different patterns */
    
    /* 1. Simple bitfield assignment via pointer - likely ZERO_EXTRACT */
    set_bitfield_via_pointer(&local_bitfield, 0x55AA);
    
    /* 2. Volatile bitfield */
    set_volatile_bitfield();
    
    /* 3. Partial register operations */
    partial_register_operations();
    
    /* 4. Complex loop with bitfields - increases chance of scheduling analysis */
    complex_bitfield_operations(5, dynamic_bitfield);
    
    /* 5. Unpredictable assignments */
    unpredictable_bitfield_set(&global_bitfield, condition);
    
    /* 6. Direct bitfield assignment to global */
    global_bitfield.field1 = 3;
    global_bitfield.field2 = 0xA;
    
    /* 7. Mixed operations in a loop */
    for (int i = 0; i < 3; i++) {
        /* Pointer arithmetic to different bitfields */
        struct BitfieldStruct *p = dynamic_bitfield + i;
        p->field1 = i & 0x7;
        p->field2 = (i * 2) & 0x1F;
        
        /* Inline assembly with memory constraints */
        asm volatile(
            "lock orb $0x1, %0\n\t"
            : "+m" (p->field3)
            :
            : "memory"
        );
    }
    
    /* Use the results to prevent dead code elimination */
    volatile int result = local_bitfield.field1 + 
                         local_bitfield.field2 + 
                         local_bitfield.field3;
    
    free(dynamic_bitfield);
    
    return result > 0 ? 0 : 1;
}
