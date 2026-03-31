/* reload_coverage.c - Program to trigger multiple reload types in GCC reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char padding[3];
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main() {
    /* Declare variables with different types and storage */
    int arr[100];
    long *ptr_arr[50];
    char buffer[256];
    double doubles[20];
    volatile int vol_int = 42;
    struct misaligned_data packed;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
        if (i < 50) ptr_arr[i] = (long*)&arr[i * 2];
        if (i < 20) doubles[i] = i * 1.5;
    }
    
    /* Take addresses to force spill/reload */
    int *addr1 = &arr[10];
    int *addr2 = &arr[20];
    int *addr3 = &arr[30];
    long *laddr = (long*)&arr[40];
    
    /* Complex loop with varying constraints */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7 % 100;
        int scale = 1 + (iter % 4);
        
        /* VARYING CONSTRAINT 1: Complex addressing with multiple memory operands
           Likely triggers: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]\n\t"
            : [out1] "=r" (arr[idx]),
              [tmp1] "=&r" (vol_int)  /* Early clobber */
            : [val1] "m" (*(int*)((char*)arr + idx * 4 + scale * 2)),
              [val2] "r" (iter),
              "m" (*(int*)((char*)arr + (idx + 10) * 4 + 8))  /* Extra memory input */
            : "memory", "cc"
        );
        
        /* VARYING CONSTRAINT 2: Multiple outputs with overlapping inputs
           Likely triggers: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
        register int r1 asm ("r10");
        register int r2 asm ("r11");
        register int r3 asm ("r12");
        
        asm volatile (
            "lea (%[base], %[index], %[scale]), %[addr]\n\t"
            "mov (%[addr]), %[val1]\n\t"
            "imul %[val2], %[val1]\n\t"
            "mov %[val1], (%[dest])\n\t"
            : [val1] "=&r" (r1),      /* Early clobber output */
              [addr] "=&r" (r2),      /* Early clobber for address */
              "=m" (arr[idx + 1])     /* Memory output */
            : [base] "r" (arr),
              [index] "r" (idx),
              [scale] "i" (4),
              [val2] "r" (scale),
              [dest] "r" (&arr[idx + 1]),
              "m" (arr[idx])          /* Memory input */
            : "memory", "cc"
        );
        
        /* VARYING CONSTRAINT 3: Nested address computation
           Likely triggers: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        long complex_addr;
        asm volatile (
            "mov %[ptr], %[tmp]\n\t"
            "add $16, %[tmp]\n\t"
            "mov %[tmp], %[out]\n\t"
            : [out] "=r" (complex_addr),
              [tmp] "=&r" (r3)
            : [ptr] "m" (*(long**)((char*)ptr_arr + idx * sizeof(long*) + 4))
            : "cc"
        );
        
        /* VARYING CONSTRAINT 4: Many operands exceeding available registers
           Likely triggers: RELOAD_FOR_OTHER_ADDRESS, various others */
        asm volatile (
            "mov %[in1], %[t1]\n\t"
            "add %[in2], %[t1]\n\t"
            "mov %[t1], %[out1]\n\t"
            "mov %[in3], %[t2]\n\t"
            "sub %[in4], %[t2]\n\t"
            "mov %[t2], %[out2]\n\t"
            "imul %[in5], %[out3]\n\t"
            : [out1] "=r" (arr[idx + 2]),
              [out2] "=r" (arr[idx + 3]),
              [out3] "=r" (arr[idx + 4]),
              [t1] "=&r" (r1),
              [t2] "=&r" (r2)
            : [in1] "r" (arr[idx]),
              [in2] "r" (arr[idx + 1]),
              [in3] "m" (*(int*)((char*)arr + idx * 4 + 32)),
              [in4] "r" (iter),
              [in5] "m" (*(int*)((char*)arr + idx * 4 + 64)),
              "m" (*(int*)((char*)arr + idx * 4 + 96))  /* Extra pressure */
            : "memory", "cc"
        );
        
        /* VARYING CONSTRAINT 5: Output address reloads */
        int* output_ptr = &arr[idx + 50];
        asm volatile (
            "mov %[val], (%[destptr])\n\t"
            : "=m" (*output_ptr)      /* Memory output with complex address */
            : [val] "r" (iter * 100),
              [destptr] "r" (output_ptr)
            : "memory"
        );
        
        /* Mix data types for additional pressure */
        char* char_ptr = (char*)&packed;
        asm volatile (
            "movb %[chr], (%[ptr])\n\t"
            : "=m" (packed.c)
            : [chr] "r" ((char)iter),
              [ptr] "r" (char_ptr)
            : "memory"
        );
        
        /* Update volatile to prevent dead code elimination */
        global_counter += iter;
    }
    
    /* Compute checksum to ensure all operations have side effects */
    long checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += arr[i];
        if (i < 20) checksum += (long)doubles[i];
    }
    checksum += global_counter + vol_int;
    
    printf("Checksum: %ld\n", checksum);
    
    /* Additional complex asm with inline assembly in address computation */
    {
        int* dynamic_addr;
        asm volatile (
            "lea (%[base], %[idx], 4), %[addr]\n\t"
            : [addr] "=r" (dynamic_addr)
            : [base] "r" (arr),
              [idx] "r" (global_counter % 50)
        );
        
        /* Force reload for operand address */
        asm volatile (
            "movl $0x12345678, (%[ptr])\n\t"
            : : [ptr] "r" (dynamic_addr)
            : "memory"
        );
    }
    
    return 0;
}
