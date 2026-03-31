/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization from removing critical patterns */
static volatile int external_condition = 1;

/* Global struct with bitfield - ensures memory storage */
struct GlobalStatus {
    unsigned int ready: 1;
    unsigned int error: 2;
    unsigned int count: 5;
    unsigned int data: 8;
    unsigned int padding: 16;
};

struct GlobalStatus global_status = {0};

/* Struct passed by pointer - forces memory access */
struct DeviceReg {
    volatile unsigned int control: 4;
    unsigned int status: 3;
    unsigned int mode: 2;
    unsigned int reserved: 23;
};

/* Function that performs bitfield assignment through pointer
 * This should generate ZERO_EXTRACT with memory destination */
void set_device_control(struct DeviceReg *dev, unsigned int value) {
    /* Compiler barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Bitfield assignment to memory through pointer */
    dev->control = value & 0xF;
    
    /* Another barrier */
    asm volatile("" : : : "memory");
}

/* Function with multiple bitfield operations in loop
 * Increases chance RTL remains during resource tracking */
void update_status_fields(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Conditional bitfield assignment - prevents dead code elimination */
        if (external_condition || i % 2) {
            global_status.ready = (i & 1);
            global_status.error = ((i >> 1) & 3);
            global_status.count = (i & 0x1F);
        }
        
        /* Memory barrier between operations */
        asm volatile("" : : : "memory");
    }
}

/* Function using inline assembly with partial register constraints
 * May generate STRICT_LOW_PART patterns */
void partial_register_ops(void) {
    volatile char byte_var;
    int temp;
    
    /* Inline asm that might generate STRICT_LOW_PART for byte register */
    asm volatile(
        "movb $0x42, %0\n\t"
        "movb %0, %%al\n\t"
        "addb $1, %%al\n\t"
        "movb %%al, %1"
        : "=m" (byte_var), "=m" (global_status.data)
        : 
        : "al", "memory"
    );
    
    /* Another asm with Q constraint (byte-addressable register) */
    register char clobber asm ("al");
    asm volatile(
        "movb $0x7F, %0"
        : "=Q" (clobber)
        :
        : "memory"
    );
    
    /* Use the clobbered register value */
    temp = clobber;
    global_status.data = temp & 0xFF;
}

/* Function with atomic operations on bitfields
 * May generate complex RTL with ZERO_EXTRACT */
void atomic_bitfield_ops(struct DeviceReg *dev) {
    /* Simulate atomic OR on bitfield */
    unsigned int old_val;
    
    /* Read-modify-write sequence on bitfield */
    do {
        old_val = dev->status;
        asm volatile("" : : : "memory");
        dev->status = (old_val | 0x1) & 0x7;
        asm volatile("" : : : "memory");
    } while (0); /* Simplified - real atomic would have proper loop */
    
    /* Using __sync builtin if available */
    #ifdef __GNUC__
    {
        unsigned int *ptr = (unsigned int *)dev;
        __sync_fetch_and_or(ptr, 0x10); /* Affects bitfield area */
    }
    #endif
}

/* Complex function with mixed operations to trigger resource tracking */
void complex_resource_pattern(int argc, char **argv) {
    struct DeviceReg *dev_reg;
    struct DeviceReg local_dev = {0};
    
    /* Allocate in memory (not register) */
    dev_reg = &local_dev;
    
    /* Make control flow unpredictable to prevent optimization */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Series of operations designed to generate target RTL patterns */
    for (int i = 0; i < 10; i++) {
        /* Bitfield assignment to memory (ZERO_EXTRACT) */
        set_device_control(dev_reg, (seed + i) & 0xF);
        
        /* Inline asm that reads/writes memory */
        asm volatile(
            "movl %1, %%eax\n\t"
            "andl $0x7, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (dev_reg->status)
            : "r" (seed + i * 3)
            : "eax", "memory"
        );
        
        /* Update global bitfields */
        if (i % 3 == 0) {
            global_status.ready = 1;
        } else if (i % 3 == 1) {
            global_status.error = 2;
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Partial register operations */
    partial_register_ops();
    
    /* Update with loop */
    update_status_fields(5);
}

/* Main function that creates the necessary context */
int main(int argc, char **argv) {
    struct DeviceReg dev_instance = {0};
    
    /* Initialize with unpredictable values */
    int init_val = argc;
    dev_instance.control = init_val & 0xF;
    dev_instance.status = (init_val >> 4) & 0x7;
    
    /* Call functions that should generate target RTL patterns */
    complex_resource_pattern(argc, argv);
    
    /* Atomic operations */
    atomic_bitfield_ops(&dev_instance);
    
    /* Additional bitfield operations through pointer */
    set_device_control(&dev_instance, 0xA);
    
    /* Force use of results to prevent elimination */
    if (global_status.ready) {
        return dev_instance.control + dev_instance.status;
    }
    
    return 0;
}
