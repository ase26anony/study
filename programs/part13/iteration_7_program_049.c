/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0L;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline, returns_twice)) 
use_many_regs(int a, int b, long c, void *d) {
    __asm__ volatile ("# dummy asm" : : "r"(a), "r"(b), "r"(c), "r"(d));
    return a + b;
}

int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int mod = argc > 2 ? atoi(argv[2]) : 7;
    
    /* Volatile sink to prevent optimizations */
    volatile int sink = 0;
    
    /* Outer loop to create hot region */
    for (int outer = 0; outer < 1000; outer++) {
        /* Many local variables creating register pressure */
        int var1 = base + outer;          /* Derived from arg + loop counter */
        int var2 = var1 * 2;              /* Arithmetic operation */
        int var3 = var2 << 3;             /* Shift operation */
        long var4 = global_const + outer; /* Global constant + variant */
        int var5 = mod * 17;              /* Constant derived from arg */
        int var6 = var5 & 0xFF;           /* Mask operation */
        void *ptr1 = &global_array[var1 & 0xFF]; /* Address computation */
        void *ptr2 = ptr1 + 8;            /* Pointer arithmetic */
        int var7 = (int)(uintptr_t)ptr2;  /* Cast to int */
        int var8 = var7 | 0x80000000;     /* Bitwise operation */
        long var9 = var4 ^ 0x55555555;    /* Bitwise with long */
        int var10 = ~var3;                /* Complement */
        int var11 = var10 + 1;            /* Increment */
        int var12 = -var2;                /* Negation */
        int var13 = var12 * var11;        /* Multiplication */
        long var14 = var9 / 3;            /* Division */
        int var15 = var13 % 19;           /* Modulo */
        void *ptr3 = &&label1;            /* Code address */
        int var16 = (int)(uintptr_t)ptr3; /* Address to int */
        int var17 = var16 >> 4;           /* Shift */
        
        /* Create multiple basic blocks with conditional branches */
        if (var1 & 1) {
            /* Branch 1 - more computations */
            int b1_var1 = var2 + var3;
            int b1_var2 = b1_var1 * 5;
            long b1_var3 = var4 - 0x1000;
            void *b1_ptr = &global_array[b1_var2 & 0xFF];
            int b1_var4 = (int)(uintptr_t)b1_ptr;
            
            /* Use in arithmetic */
            var17 += b1_var4;
            var15 ^= b1_var2;
            
            /* Call to clobber registers */
            clobber_registers();
            
            /* More computations after call */
            int b1_var5 = b1_var4 * 3;
            int b1_var6 = b1_var5 / 7;
            var8 = b1_var6 + var17;
        } else {
            /* Branch 2 - different computations */
            int b2_var1 = var5 * var6;
            long b2_var2 = var14 + 0x2000;
            int b2_var3 = b2_var1 & 0x7FFFFFFF;
            void *b2_ptr = ptr2 - 16;
            int b2_var4 = (int)(uintptr_t)b2_ptr;
            
            /* Different operations */
            var17 -= b2_var4;
            var15 |= b2_var3;
            
            /* Another register-clobbering call */
            int tmp = use_many_regs(var17, b2_var3, b2_var2, b2_ptr);
            var8 = tmp * 2;
            
            /* More values to keep live */
            int b2_var5 = b2_var4 << 2;
            int b2_var6 = b2_var5 ^ 0xAAAAAAAA;
            var16 = b2_var6;
        }
        
    label1:
        /* Third basic block - merge point with more computations */
        int var18 = var8 + var15;
        int var19 = var17 * var16;
        long var20 = var14 + var9;
        int var21 = var18 & var19;
        int var22 = var21 | var1;
        void *ptr4 = &global_array[var22 & 0xFF];
        int var23 = (int)(uintptr_t)ptr4;
        int var24 = var23 * 3;
        int var25 = var24 / 5;
        long var26 = var20 - global_const;
        int var27 = var25 % 13;
        int var28 = var27 << 1;
        int var29 = ~var28;
        int var30 = var29 + var22;
        
        /* Another conditional to create more blocks */
        if (var30 & 2) {
            int c1_var1 = var26 & 0xFFFFFFFF;
            int c1_var2 = c1_var1 + var30;
            var19 = c1_var2 * 11;
            
            /* Force spill/reload around call */
            clobber_registers();
            
            int c1_var3 = var19 ^ 0xCCCCCCCC;
            var23 = c1_var3;
        } else {
            int c2_var1 = (int)(var26 >> 32);
            int c2_var2 = c2_var1 - var30;
            var19 = c2_var2 / 3;
            
            /* Different call pattern */
            int tmp = use_many_regs(var19, c2_var2, var26, ptr4);
            var23 = tmp;
        }
        
        /* Final computations using many values */
        int var31 = var19 + var23;
        int var32 = var31 * var30;
        int var33 = var32 / 7;
        int var34 = var33 ^ var22;
        long var35 = var26 + var34;
        int var36 = (int)var35;
        int var37 = var36 & 0x7FFF;
        int var38 = var37 | 0x8000;
        int var39 = var38 * var1;
        int var40 = var39 % 17;
        
        /* Use results to prevent elimination */
        sink += var40;
        global_array[outer & 0xFF] = var40;
        
        /* Additional loop-variant constants */
        int const1 = base + (outer << 2);      /* Varying constant */
        int const2 = const1 * 3;               /* Derived constant */
        long const3 = global_const + const2;   /* Global + variant */
        void *const4 = &global_array[const2 & 0xFF]; /* Address computation */
        int const5 = (int)(uintptr_t)const4;   /* Address to int */
        
        /* Use these constants in final computation */
        var40 += const5;
        sink ^= const2;
        
        /* One more register-clobbering operation */
        __asm__ volatile ("# final asm clobber" 
                         : "+r"(var40), "+r"(const5)
                         : "r"(const3), "r"(const4)
                         : "memory", "cc", 
                           "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
    }
    
    return sink;
}
