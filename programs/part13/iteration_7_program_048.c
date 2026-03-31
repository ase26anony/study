/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early_remat_test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0LL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y, void *ptr) {
    __asm__ volatile ("# dummy asm" : : "r"(x), "r"(y), "r"(ptr));
    return x + (int)(y & 0xFFFFFFFF);
}

/* Complex computation to generate many intermediate values */
static long __attribute__((noinline)) compute_remat_candidate(int base, int idx) {
    /* These constants are rematerialization candidates */
    const long magic1 = 0xDEADBEEFCAFEBABELL;
    const int magic2 = 0x12345678;
    const short magic3 = 0xABCD;
    
    /* Address computation - another remat candidate */
    void *addr1 = &global_array[idx % 256];
    void *addr2 = &global_array[(idx + 1) % 256];
    
    /* Multiple intermediate values with different types */
    long val1 = base + magic1;          /* Constant + arg */
    int val2 = base * magic2;           /* Constant * arg */
    short val3 = (short)(magic3 + idx); /* Constant + loop var */
    long val4 = (long)addr1 - (long)addr2; /* Pointer difference */
    int val5 = val2 << 2;               /* Shift operation */
    long val6 = val1 | val4;            /* Bitwise OR */
    
    /* Force all values to be used */
    __asm__ volatile ("# compute values" : : 
                     "r"(val1), "r"(val2), "r"(val3), "r"(val4), "r"(val5), "r"(val6));
    
    return val1 + val2 + val3 + val4 + val5 + val6;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    int param_base = (argc > 1) ? atoi(argv[1]) : 42;
    int param_shift = (argc > 2) ? atoi(argv[2]) : 3;
    
    volatile int sink = 0; /* Prevent dead code elimination */
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Inner loop with many basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            /* Small integer constants (require multiple instructions) */
            const int const1 = 0x7FFFFFFF;  /* Large immediate */
            const long const2 = 0x1234567890ABCDEFLL; /* 64-bit constant */
            const int const3 = 0x55555555;  /* Bit pattern constant */
            
            /* Derived from function arguments (remat candidates) */
            int derived1 = param_base + 1;      /* param + small const */
            int derived2 = param_base << param_shift; /* param << const */
            long derived3 = (long)param_base * 1000L; /* param * const */
            
            /* Symbol addresses that can be recomputed */
            int *addr1 = &global_array[i % 256];
            int *addr2 = &global_array[(i + 64) % 256];
            int *addr3 = &global_array[(i + 128) % 256];
            
            /* Complex expressions as remat candidates */
            long expr1 = (long)addr1 - (long)addr2;
            int expr2 = (int)((long)addr3 & 0xFFFF);
            long expr3 = global_const + i;
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* More computations in this path */
                int temp1 = derived1 * const1;
                long temp2 = derived3 + const2;
                int temp3 = expr2 | const3;
                
                /* Use volatile to prevent optimization */
                __asm__ volatile ("# branch1" : : 
                                "r"(temp1), "r"(temp2), "r"(temp3),
                                "r"(addr1), "r"(addr2), "r"(addr3));
                
                sink += temp1 + (int)temp2 + temp3;
                
                /* Function call clobbers registers */
                clobber_registers();
                
                /* More computations after clobber */
                int restored1 = derived2 + 1;  /* Should be rematerialized */
                long restored2 = expr3 - 100;  /* Should be rematerialized */
                int restored3 = (int)expr1 >> 2; /* Should be rematerialized */
                
                /* Use in another function call */
                sink += use_value(restored1, restored2, (void*)restored3);
            } else {
                /* Alternative path with different computations */
                int temp4 = derived2 / 2;
                long temp5 = expr3 * 2;
                int temp6 = const3 ^ expr2;
                
                /* Different addressing mode */
                int *addr4 = &global_array[(i + 192) % 256];
                long offset = (long)addr4 - (long)&global_array[0];
                
                __asm__ volatile ("# branch2" : : 
                                "r"(temp4), "r"(temp5), "r"(temp6),
                                "r"(addr4), "r"(offset));
                
                sink += temp4 + (int)temp5 + temp6 + (int)offset;
                
                /* Another register-clobbering call */
                clobber_registers();
                
                /* Values that need rematerialization after clobber */
                int restored4 = derived1 - 1;
                long restored5 = expr1 + 500;
                int restored6 = const1 & 0xFF;
                
                /* Complex expression forcing register pressure */
                long complex = compute_remat_candidate(restored4, restored6);
                sink += (int)complex + restored4 + restored6;
            }
            
            /* --- Basic Block 3: Common code after branches --- */
            /* More intermediate values */
            int final1 = (derived1 + derived2) * 3;
            long final2 = (expr3 + global_const) / 2;
            int final3 = (const1 ^ const3) + i;
            
            /* Pointer arithmetic with different modes */
            char *char_ptr = (char*)addr1 + final3;
            short *short_ptr = (short*)addr2 + (final1 % 128);
            long *long_ptr = (long*)addr3 + (final1 % 64);
            
            /* Mix data types to create REGs with different modes */
            __asm__ volatile ("# mixed types" : : 
                            "r"(final1), "r"(final2), "r"(final3),
                            "r"(char_ptr), "r"(short_ptr), "r"(long_ptr));
            
            /* Store to volatile to keep values alive */
            global_counter += final1 + (int)final2 + final3;
            
            /* Another clobber to force more remat */
            clobber_registers();
            
            /* Final use of values that may need rematerialization */
            if ((i % 100) == 0) {
                /* Force spill/remat decision point */
                int check1 = derived1 + global_counter;
                long check2 = expr3 - global_counter;
                int check3 = const3 | global_counter;
                
                sink += check1 + (int)check2 + check3;
                
                /* Call with many arguments to increase pressure */
                __asm__ volatile ("# many args" : : 
                                "r"(derived1), "r"(derived2), "r"(derived3),
                                "r"(expr1), "r"(expr2), "r"(expr3),
                                "r"(const1), "r"(const2), "r"(const3),
                                "r"(addr1), "r"(addr2), "r"(addr3));
            }
        }
    }
    
    printf("Result: %d (sink=%d)\n", global_counter, sink);
    return 0;
}
