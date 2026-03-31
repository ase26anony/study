/* test_resource_coverage.c
 * Designed to trigger mark_set_resources path for ZERO_EXTRACT/STRICT_LOW_PART
 * with memory destinations in GCC's resource tracking pass.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    unsigned int ready: 1;
    unsigned int count: 4;
    unsigned int error: 2;
    unsigned int padding: 25;
};

volatile struct GlobalStatus g_status = {0, 0, 0, 0};

/* Struct passed by pointer to force memory access */
struct DeviceReg {
    unsigned int control: 3;
    unsigned int mode: 2;
    unsigned int enable: 1;
    unsigned int reserved: 26;
};

/* Function that modifies bitfield via pointer - should generate ZERO_EXTRACT */
void set_device_control(struct DeviceReg *reg, unsigned int value) {
    /* Compiler barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* This assignment should generate SET with ZERO_EXTRACT destination */
    reg->control = value & 0x7;
    
    /* Another barrier to keep operations separate */
    asm volatile("" : : : "memory");
}

/* Function with volatile bitfield - forces memory access */
void update_status(void) {
    volatile struct {
        unsigned int flag: 1;
        unsigned int state: 3;
    } local_status;
    
    /* Volatile ensures memory access, not register */
    local_status.flag = 1;
    local_status.state = (g_status.count & 0x7);
}

/* Function with inline assembly that might generate STRICT_LOW_PART */
void partial_register_ops(void) {
    int temp;
    
    /* Using byte-addressable register constraint "=Q" 
     * May generate STRICT_LOW_PART on some architectures */
    __asm__ volatile(
        "movb $0x42, %%al\n\t"
        "movb %%al, %0"
        : "=Q" (temp)
        : 
        : "al"
    );
    
    /* Memory reference with byte operation */
    char *mem_byte = (char *)&g_status;
    __asm__ volatile(
        "orb $0x01, %0"
        : "+m" (*mem_byte)
        :
        : "cc"
    );
}

/* Complex function with multiple bitfield operations in loop */
void process_bitfields(int iterations) {
    struct DeviceReg regs[2];
    int i;
    
    /* Initialize with external input to prevent dead code elimination */
    for (i = 0; i < iterations && i < 2; i++) {
        /* Multiple bitfield assignments in loop */
        regs[i].control = (i * 3) & 0x7;
        regs[i].mode = (i + 1) & 0x3;
        regs[i].enable = i & 0x1;
        
        /* Mix with inline assembly to create scheduling complexity */
        __asm__ volatile(
            "mfence\n\t"
            : : : "memory"
        );
    }
    
    /* Use atomic operation on bitfield - may generate complex RTL */
    struct DeviceReg * volatile p_reg = &regs[0];
    unsigned int old_val;
    
    /* Simulate atomic update of bitfield */
    do {
        old_val = *(volatile unsigned int *)p_reg;
        unsigned int new_val = (old_val & ~(0x7 << 0)) | ((old_val + 1) & 0x7);
        
        /* This compare-and-swap pattern may preserve bitfield RTL */
        __asm__ volatile(
            "lock; cmpxchgl %2, %1"
            : "=a" (old_val), "+m" (*(volatile unsigned int *)p_reg)
            : "r" (new_val), "0" (old_val)
            : "cc", "memory"
        );
    } while (0);
}

/* Function with unpredictable control flow to prevent optimization */
void conditional_bitfield_update(int condition) {
    static struct DeviceReg static_reg;
    
    /* External condition prevents dead code elimination */
    if (condition & 0x1) {
        static_reg.control = 1;
        static_reg.mode = 2;
    } else {
        static_reg.control = 3;
        static_reg.mode = 1;
    }
    
    /* Force memory barrier */
    __sync_synchronize();
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    struct DeviceReg local_reg = {0, 0, 0, 0};
    int i;
    
    /* Use argc to make control flow unpredictable */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations > 100) iterations = 100;
    
    /* 1. Bitfield assignment via pointer (ZERO_EXTRACT to memory) */
    set_device_control(&local_reg, 5);
    
    /* 2. Global volatile bitfield update */
    g_status.ready = 1;
    g_status.count = (iterations & 0xF);
    
    /* 3. Update status with volatile local */
    update_status();
    
    /* 4. Partial register operations */
    partial_register_ops();
    
    /* 5. Complex bitfield processing with loop */
    process_bitfields(iterations);
    
    /* 6. Conditional updates based on input */
    for (i = 0; i < iterations; i++) {
        conditional_bitfield_update(i);
        
        /* Mix with memory operations to create scheduling opportunities */
        __asm__ volatile(
            "movl %0, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            "movl %%eax, %1"
            : 
            : "m" (g_status), "m" (local_reg)
            : "eax", "memory"
        );
    }
    
    /* Use __sync builtin on bitfield */
    struct DeviceReg *reg_ptr = &local_reg;
    unsigned int *int_ptr = (unsigned int *)reg_ptr;
    __sync_fetch_and_or(int_ptr, 0x1);
    
    /* Final output to prevent entire program from being optimized away */
    printf("Result: control=%d, ready=%d\n", 
           local_reg.control, g_status.ready);
    
    return 0;
}
