/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main() {
    /* Declare variables with different types and storage */
    int arr[256];
    long *ptr_arr[16];
    struct misaligned_data packed;
    volatile int vol_var = 0;
    register int reg_var asm("r12") = 42; /* Suggest register */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        ptr_arr[i] = (long*)&arr[i * 16];
    }
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE12345678ULL;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 100; iter++) {
        int idx = iter % 256;
        int idx2 = (iter * 7) % 256;
        int idx3 = (iter * 13) % 256;
        
        /* Vary constraints based on iteration to trigger different reloads */
        int constraint_type = iter % 8;
        
        switch (constraint_type) {
            case 0:
                /* Complex addressing modes - RELOAD_FOR_INPUT_ADDRESS */
                asm volatile (
                    "add %[out], %[in1], %[in2], lsl #2\n\t"
                    : [out] "=r" (arr[idx])
                    : [in1] "r" (arr[idx2]), 
                      [in2] "r" (arr[idx3]),
                      "m" (arr[(idx + idx2) % 256]),  /* Memory operand */
                      "m" (arr[(idx * 3) % 256])      /* Another memory operand */
                    : "cc", "memory"
                );
                break;
                
            case 1:
                /* Multiple operands with register pressure - RELOAD_FOR_INPUT/OUTPUT */
                asm volatile (
                    "mov %[o1], %[i1]\n\t"
                    "add %[o2], %[i2], %[i3]\n\t"
                    "mul %[o3], %[i4], %[i5]\n\t"
                    : [o1] "=&r" (arr[idx]),      /* Early clobber */
                      [o2] "=&r" (arr[idx2]),     /* Early clobber */
                      [o3] "=r" (arr[idx3])
                    : [i1] "r" (arr[(idx + 1) % 256]),
                      [i2] "r" (arr[(idx2 + 2) % 256]),
                      [i3] "r" (arr[(idx3 + 3) % 256]),
                      [i4] "r" (arr[(idx + 4) % 256]),
                      [i5] "r" (arr[(idx2 + 5) % 256]),
                      "m" (packed),               /* Memory operand */
                      "m" (vol_var)               /* Volatile memory */
                    : "cc", "memory"
                );
                break;
                
            case 2:
                /* Nested address computation - RELOAD_FOR_OPERAND_ADDRESS */
                {
                    long *ptr1 = &arr[idx];
                    long *ptr2 = &arr[idx2];
                    long *ptr3 = &arr[idx3];
                    
                    asm volatile (
                        "ldr x0, [%[p1], %[off1], lsl #3]\n\t"
                        "str x0, [%[p2], %[off2], lsl #3]\n\t"
                        "add %[res], %[p3], %[off3]\n\t"
                        : [res] "=r" (ptr_arr[iter % 16])
                        : [p1] "r" (ptr1),
                          [off1] "r" (idx),
                          [p2] "r" (ptr2),
                          [off2] "r" (idx2),
                          [p3] "r" (ptr3),
                          [off3] "r" (idx3 * sizeof(int)),
                          "m" (*ptr1),            /* Memory operand */
                          "m" (*ptr2)             /* Another memory operand */
                        : "x0", "cc", "memory"
                    );
                }
                break;
                
            case 3:
                /* Output address reloads - RELOAD_FOR_OUTPUT_ADDRESS */
                {
                    int *out_ptr = &arr[idx];
                    
                    asm volatile (
                        "stp %[v1], %[v2], [%[ptr], #16]\n\t"
                        : "=m" (*out_ptr)
                        : [v1] "r" (iter),
                          [v2] "r" (iter + 1),
                          [ptr] "r" (out_ptr),
                          "m" (arr[idx2]),        /* Input memory */
                          "m" (arr[idx3])         /* Another input */
                        : "memory"
                    );
                }
                break;
                
            case 4:
                /* Mixed data types and alignment - RELOAD_FOR_OTHER_ADDRESS */
                {
                    char *cptr = (char*)&packed;
                    int *iptr = (int*)(cptr + 1); /* Misaligned! */
                    
                    asm volatile (
                        "ldrb w1, [%[cp]]\n\t"
                        "ldr w2, [%[ip]]\n\t"
                        "add w1, w1, w2\n\t"
                        "strb w1, [%[cp]]\n\t"
                        : 
                        : [cp] "r" (cptr),
                          [ip] "r" (iptr),
                          "m" (packed.c),
                          "m" (packed.i)
                        : "w1", "w2", "cc", "memory"
                    );
                }
                break;
                
            case 5:
                /* Input address address - RELOAD_FOR_INPADDR_ADDRESS */
                {
                    int *addr_of_mem = &arr[idx];
                    
                    asm volatile (
                        "ldr x0, [%[addr]]\n\t"
                        "add x0, x0, #1\n\t"
                        "str x0, [%[addr]]\n\t"
                        : 
                        : [addr] "r" (&addr_of_mem), /* Address of address */
                          "m" (addr_of_mem),         /* Memory containing address */
                          "m" (arr[idx])             /* The actual data */
                        : "x0", "cc", "memory"
                    );
                }
                break;
                
            case 6:
                /* Output address address - RELOAD_FOR_OUTADDR_ADDRESS */
                {
                    int **out_addr_ptr = &ptr_arr[iter % 8];
                    
                    asm volatile (
                        "mov x0, %[val]\n\t"
                        "str x0, [%[ptr]]\n\t"
                        : "=m" (*out_addr_ptr)
                        : [val] "r" (&arr[idx]),
                          [ptr] "r" (out_addr_ptr),
                          "m" (ptr_arr[iter % 8])
                        : "x0", "memory"
                    );
                }
                break;
                
            case 7:
                /* Operand address address - RELOAD_FOR_OPADDR_ADDR */
                {
                    int (*func_ptr)(void) = (int (*)(void))&arr[idx];
                    
                    asm volatile (
                        "blr %[fn]\n\t"
                        : 
                        : [fn] "r" (func_ptr),
                          "m" (arr[idx]),          /* Code as data */
                          "r" (reg_var)            /* Register variable */
                        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
                          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                          "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
                          "x24", "x25", "x26", "x27", "x28", "lr", "cc", "memory"
                    );
                }
                break;
        }
        
        /* Force register pressure by using many variables */
        vol_var += iter;
        reg_var ^= arr[idx];
        global_counter++;
        
        /* Complex expression with addressing */
        arr[(idx + 64) % 256] = arr[idx] + arr[idx2] * arr[idx3] - vol_var;
    }
    
    /* Compute checksum to ensure all operations have side effects */
    unsigned long long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += arr[i];
        checksum ^= (unsigned long long)ptr_arr[i % 16] << (i % 64);
    }
    checksum += packed.i + packed.l + vol_var + reg_var + global_counter;
    
    printf("Checksum: %llx\n", checksum);
    printf("Final values: arr[0]=%d, arr[255]=%d, vol_var=%d, reg_var=%d\n",
           arr[0], arr[255], vol_var, reg_var);
    
    return (int)(checksum & 0x7FFFFFFF);
}
