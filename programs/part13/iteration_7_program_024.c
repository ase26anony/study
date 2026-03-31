/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o trigger early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_offset = 2000;
int global_array[256] = {0};
static volatile int sink;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) compute_offset(int idx) {
    return idx * 7 + 3;  /* Non-trivial constant expression */
}

int main(int argc, char *argv[]) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    int param = (argc > 2) ? atoi(argv[2]) : 42;
    
    volatile int result = 0;
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < iterations; outer++) {
        /* Create many short-lived intermediate values with different modes */
        
        /* Integer constants (SImode candidates) */
        int const1 = 0x7FFFFFFF;  /* Large constant requiring multiple insns */
        int const2 = 0x12345678;
        int const3 = -987654321;
        int const4 = 0xDEADBEEF;
        
        /* Long constants (DImode candidates on 64-bit) */
        long const5 = 0x7FFFFFFFFFFFFFFFL;
        long const6 = 0x123456789ABCDEF0L;
        
        /* Derived constants from parameters */
        int derived1 = param + 0x1000;      /* param + constant */
        int derived2 = param << 4;          /* param shifted */
        int derived3 = param * 13;          /* param multiplied */
        int derived4 = ~param;              /* param inverted */
        
        /* Address calculations (Pmode candidates) */
        int *addr1 = &global_array[outer & 0xFF];
        int *addr2 = &global_array[(outer + 1) & 0xFF];
        int *addr3 = &global_array[(outer * 3) & 0xFF];
        
        /* Loop-variant expressions */
        int variant1 = outer * 17 + 5;
        int variant2 = (outer << 3) | 0xF;
        int variant3 = compute_offset(outer);
        
        /* Force use of all these values in a complex control flow */
        if (outer & 1) {
            /* Branch 1: Use integer constants */
            result += const1 + const2;
            result -= const3 ^ const4;
            
            /* Mix with derived constants */
            int temp1 = derived1 * derived2;
            int temp2 = derived3 | derived4;
            result += temp1 - temp2;
            
            /* Use address calculations */
            *addr1 = variant1;
            *addr2 = variant2;
            
            clobber_registers();  /* Force register clobbering */
        } else {
            /* Branch 2: Use long constants */
            long ltemp = const5 + const6;
            result += (int)(ltemp >> 32);
            
            /* More complex expressions */
            int temp3 = (variant1 * variant2) / (variant3 + 1);
            int temp4 = (derived1 << 2) + (derived2 >> 1);
            result += temp3 * temp4;
            
            /* Use the third address */
            *addr3 = global_base + outer;
            
            clobber_registers();  /* Force register clobbering */
        }
        
        /* Additional basic block with more computations */
        if (outer & 2) {
            /* Create more intermediate values */
            int chain1 = result + global_offset;
            int chain2 = chain1 * 3;
            int chain3 = chain2 - param;
            int chain4 = chain3 ^ 0x55555555;
            
            /* Pointer arithmetic with different scales */
            char *byte_ptr = (char*)addr1 + outer;
            int *int_ptr = (int*)((long)addr2 + (outer * 4));
            
            /* Force these to be used */
            result = chain4 + *byte_ptr + *int_ptr;
            
            clobber_registers();
        } else {
            /* Alternative path with different computations */
            long lchain1 = (long)result * const5;
            int ichain2 = (int)(lchain1 & 0xFFFFFFFF);
            int ichain3 = ichain2 + global_base;
            int ichain4 = ichain3 - derived1;
            
            /* More address calculations */
            int *addr4 = &global_array[(outer + 5) & 0xFF];
            int *addr5 = addr4 + (param & 0xF);
            
            result = ichain4 + *addr5;
            
            clobber_registers();
        }
        
        /* Final computation block mixing everything */
        int final1 = result + const1;
        int final2 = final1 - (derived2 >> 2);
        long final3 = (long)final2 * const6;
        int final4 = (int)(final3 ^ const5);
        
        /* Store to volatile to prevent elimination */
        sink = final4 + (int)((long)addr1 & 0xFF);
        
        /* Use inline asm to prevent optimization */
        __asm__ volatile ("# marker %0 %1 %2" : : "r"(final4), "r"(addr2), "r"(param) : "memory");
    }
    
    return sink & 0xFF;
}
