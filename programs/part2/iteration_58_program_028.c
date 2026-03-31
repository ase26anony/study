/* test_resource_coverage.c
 * This program is designed to trigger uncovered lines in GCC's resource.cc
 * Specifically lines 282-290 in mark_set_resources function
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* ========== BITFIELD STRUCTURES FOR ZERO_EXTRACT ========== */

/* Global struct with bitfield - ensures memory storage */
struct GlobalStatus {
    volatile unsigned int ready : 1;    /* volatile forces memory access */
    unsigned int count : 7;             /* 7-bit field */
    unsigned int mode : 4;              /* 4-bit field */
    unsigned int padding : 20;          /* padding to 32 bits */
} global_status;

/* Another struct for pointer-based access */
struct DeviceReg {
    unsigned int control : 8;
    unsigned int status : 8;
    unsigned int data : 16;
};

/* Volatile pointer to mimic hardware register */
volatile struct DeviceReg *device_reg;

/* ========== FUNCTIONS TO GENERATE SPECIFIC PATTERNS ========== */

/* Function 1: Bitfield assignment through pointer - should generate ZERO_EXTRACT with MEM */
void set_bitfield_via_pointer(struct GlobalStatus *status, int value) {
    /* Multiple bitfield assignments to increase pattern visibility */
    status->count = value & 0x7F;      /* 7-bit field */
    status->mode = (value >> 7) & 0xF; /* 4-bit field */
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* Conditional assignment based on external input */
    if (value & 1) {
        status->ready = 1;
    } else {
        status->ready = 0;
    }
}

/* Function 2: Complex bitfield operations with inline assembly */
void bitfield_with_asm_operations(int param) {
    struct DeviceReg reg;
    struct DeviceReg *reg_ptr = &reg;
    
    /* Initialize */
    reg.control = 0;
    reg.status = 0;
    reg.data = 0;
    
    /* Inline assembly that forces memory reference */
    asm volatile(
        "movl %[input], %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movb %%al, %[control]\n\t"
        : [control] "=m" (reg_ptr->control)  /* Memory constraint for bitfield */
        : [input] "r" (param)
        : "eax", "memory"
    );
    
    /* More bitfield operations */
    reg_ptr->status = (param >> 8) & 0xFF;
    
    /* Atomic operation on bitfield - may generate complex RTL */
    int old_val;
    do {
        old_val = reg_ptr->data;
    } while (!__sync_bool_compare_and_swap(
        (int*)&reg_ptr->data, 
        old_val, 
        (old_val & 0xFFFF) | ((param & 0xFFFF) << 16)
    ));
}

/* Function 3: STRICT_LOW_PART pattern using inline assembly */
void strict_low_part_pattern(void) {
    /* Using byte-addressable register constraint "=Q" */
    register char byte_reg asm("al");
    
    /* Multiple asm statements to create scheduling opportunities */
    asm volatile(
        "movb $0x55, %0\n\t"
        : "=Q" (byte_reg)
        :
        : "memory"
    );
    
    /* Memory reference after STRICT_LOW_PART */
    volatile char memory_byte;
    asm volatile(
        "movb %1, %%al\n\t"
        "movb %%al, %0\n\t"
        : "=m" (memory_byte)
        : "Q" (byte_reg)
        : "al", "memory"
    );
    
    /* Another asm with different constraints */
    uint32_t dword;
    asm volatile(
        "movl $0x12345678, %%eax\n\t"
        "movb %%al, %0\n\t"  /* Only modifies low byte */
        : "=m" (*(volatile char*)&dword)
        :
        : "eax", "memory"
    );
}

/* Function 4: Loop with bitfield assignments - prevents optimization */
void loop_with_bitfield_ops(int iterations) {
    struct {
        unsigned int flags : 3;
        unsigned int state : 5;
        unsigned int counter : 10;
    } local_data;
    
    volatile int i;  /* volatile to prevent loop unrolling */
    for (i = 0; i < iterations; i++) {
        /* Multiple bitfield assignments in loop */
        local_data.flags = i & 0x7;
        local_data.state = (i >> 3) & 0x1F;
        local_data.counter = i & 0x3FF;
        
        /* External function call prevents dead code elimination */
        if (i % 100 == 0) {
            putchar('.');
        }
    }
}

/* Function 5: Mixed operations to trigger resource tracking */
void mixed_resource_operations(int seed) {
    /* Global bitfield access */
    global_status.ready = seed & 1;
    
    /* Pointer to local struct with bitfields */
    struct {
        unsigned int a : 2;
        unsigned int b : 6;
        unsigned int c : 8;
    } local, *local_ptr = &local;
    
    /* Assign through pointer - should generate ZERO_EXTRACT(MEM) */
    local_ptr->a = (seed >> 1) & 0x3;
    local_ptr->b = (seed >> 3) & 0x3F;
    local_ptr->c = (seed >> 9) & 0xFF;
    
    /* Inline assembly that clobbers registers */
    asm volatile(
        "pushf\n\t"
        "popf\n\t"  /* Serializing instruction */
        : : : "memory", "cc"
    );
    
    /* Sync operation on bitfield */
    unsigned int old;
    do {
        old = local_ptr->c;
    } while (!__sync_bool_compare_and_swap(
        (unsigned int*)&local_ptr->c,
        old,
        (old + 1) & 0xFF
    ));
}

/* ========== MAIN FUNCTION ========== */

int main(int argc, char *argv[]) {
    /* Use argc to create unpredictable control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : 0x1234;
    
    printf("Testing resource tracking patterns...\n");
    
    /* Initialize device register pointer */
    struct DeviceReg reg;
    device_reg = &reg;
    
    /* Call functions that generate different patterns */
    
    /* 1. ZERO_EXTRACT pattern with memory reference */
    set_bitfield_via_pointer(&global_status, seed);
    
    /* 2. Bitfield with inline assembly */
    bitfield_with_asm_operations(seed);
    
    /* 3. STRICT_LOW_PART pattern */
    strict_low_part_pattern();
    
    /* 4. Loop with bitfield ops - prevents optimization */
    loop_with_bitfield_ops(iterations % 1000);
    
    /* 5. Mixed operations */
    mixed_resource_operations(seed ^ 0xABCD);
    
    printf("\nDone.\n");
    
    /* Return something based on the operations */
    return (global_status.ready + 
            reg.control + 
            reg.status + 
            reg.data) & 0xFF;
}
