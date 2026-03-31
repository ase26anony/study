/* auto_inc_dec_trigger.c
 * Designed to trigger auto-increment/decrement RTL pattern matching
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

/* Prevent inlining of helper functions */
#define NOINLINE __attribute__((noinline))

/* Memory barrier to prevent optimization */
#define MEMORY_BARRIER() __asm__ volatile("" : : : "memory")

/* Register clobbering barrier for ARM */
#ifdef __arm__
#define REG_BARRIER() __asm__ volatile("" : : : "r0", "r1", "r2", "r3", "memory")
#else
#define REG_BARRIER() __asm__ volatile("" : : : "memory")
#endif

/* Dummy function to prevent dead code elimination */
NOINLINE void consume_data(volatile int* data, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
        sink += data[i];
    }
    /* Prevent optimization of sink */
    __asm__ volatile("" : "+r" (sink));
}

/* Another dummy function to create register pressure */
NOINLINE int process_value(int val) {
    return val * 3 + 7;
}

int main(void) {
    /* Volatile arrays to prevent optimization of memory accesses */
    volatile int src[100];
    volatile int dst[100];
    
    /* Initialize source array with sequential values */
    for (int i = 0; i < 100; i++) {
        src[i] = i * 2 + 1;
    }
    
    /* Non-constant loop bound to prevent unrolling */
    int limit = 90;
    volatile int accumulator = 0;
    
    /* 
     * CRITICAL LOOP: Designed to generate (mem (plus (reg) (const_int 0))) patterns
     * Multiple memory references with constant index 0
     */
    for (int i = 0; i < limit; i++) {
        /* Pattern 1: Load from src[0] - should generate (mem (reg)) */
        int val1 = src[0];  /* Constant index 0 */
        
        /* Pattern 2: Load from src[1] - another base register */
        int val2 = src[1];  /* Another constant index */
        
        /* Arithmetic operation to create register dependencies */
        int result = val1 + val2 + i;
        
        /* Pattern 3: Store to dst[0] - (set (mem (reg)) (reg)) */
        dst[0] = result;    /* Constant index 0 */
        
        /* Memory barrier to prevent reordering/elimination */
        MEMORY_BARRIER();
        
        /* Pattern 4: Pointer dereference with zero offset */
        volatile int *p = &src[10];
        int val3 = *p;      /* Dereference: *(reg + 0) */
        
        /* Pattern 5: Store to another array location */
        dst[5] = val3 + result;
        
        /* Register pressure barrier */
        REG_BARRIER();
        
        /* Scalar operation that gets stored back */
        accumulator += result;
        dst[2] = accumulator;  /* Another memory store */
        
        /* Additional memory access with constant 0 offset */
        int temp = src[2];  /* src[2] = *(base_reg + 8) on most arches */
        dst[3] = process_value(temp);
        
        /* Force register reload */
        MEMORY_BARRIER();
    }
    
    /* 
     * Additional pointer-based accesses to create more (mem (reg)) patterns
     * These use different base registers
     */
    volatile int *ptr1 = &src[20];
    volatile int *ptr2 = &dst[30];
    
    for (int j = 0; j < 10; j++) {
        /* Multiple loads with the same base+0 pattern */
        int a = *ptr1;      /* (mem (reg)) */
        int b = *ptr2;      /* (mem (reg)) with different base */
        
        /* Simple arithmetic */
        int c = a + b;
        
        /* Store back through pointers */
        *ptr1 = c;          /* (set (mem (reg)) (reg)) */
        *ptr2 = c * 2;
        
        /* Barrier to separate memory operations */
        REG_BARRIER();
        
        /* Access with constant 0 offset via array syntax */
        int d = src[j];     /* Will become (mem (plus (reg) (const_int 0))) after optimization */
        dst[j + 40] = d;
    }
    
    /* Mixed scalar and array operations in another loop */
    volatile int counter = 0;
    for (int k = 0; k < 50; k++) {
        /* Load-modify-store pattern */
        int base_val = src[k];
        int modified = base_val + counter;
        dst[k + 50] = modified;
        
        /* Update scalar */
        counter = modified % 100;
        
        /* Access with pointer arithmetic that simplifies to base+0 */
        volatile int *ptr = &src[k + 30];
        int ptr_val = *ptr;  /* (mem (reg)) */
        dst[k + 60] = ptr_val;
        
        /* Occasional barrier */
        if (k % 7 == 0) {
            MEMORY_BARRIER();
        }
    }
    
    /* Prevent dead code elimination */
    consume_data(dst, 100);
    
    /* Calculate checksum to ensure all operations matter */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= dst[i];  /* XOR checksum */
        checksum += src[i];  /* Add source values */
    }
    
    return checksum & 0xFF;  /* Return non-zero result */
}
