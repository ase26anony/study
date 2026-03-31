/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
volatile int result_index = 0;

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed * 1;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 4 + 3;
    i5 = seed * 5 + 4;
    
    /* Initialize long doubles */
    ld1 = (long double)(seed + 1) / 3.1415926535L;
    ld2 = (long double)(seed + 2) / 2.7182818284L;
    ld3 = (long double)(seed + 3) / 1.4142135623L;
    ld4 = (long double)(seed + 4) / 1.6180339887L;
    
    /* Force use of rdtsc which uses fixed registers (eax, edx) */
    {
        uint32_t lo, hi;
        __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
        i6 = lo + hi;
    }
    
    /* Mix integer operations to create register pressure */
    i7 = i1 + i2;
    i8 = i3 * i4;
    i9 = i5 ^ i6;
    i10 = i7 - i8;
    
    /* Critical section: x87 operations with constraints that may need secondary reloads */
    
    /* 1. Simple x87 operation with "t" constraint (top of x87 stack) */
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "faddp %%st(1), %%st\n\t"
        "fstpt %0"
        : "=m"(ld5)
        : "m"(ld1), "m"(ld2)
        : "st", "st(1)"
    );
    
    /* 2. More complex: mixing x87 with integer via conversion */
    ld6 = ld3;
    __asm__ volatile (
        "fildl %1\n\t"          /* Load integer to x87 stack */
        "fldt %2\n\t"           /* Load long double */
        "fmulp %%st(1), %%st\n\t"
        "fstpt %0"
        : "=m"(ld7)
        : "m"(i7), "m"(ld4)
        : "st", "st(1)"
    );
    
    /* 3. Multi-alternative constraint: "rm,t" - may force secondary reload */
    /* This is the key pattern to trigger uncovered lines */
    ld8 = ld2;
    int temp_int = i8 + seed;
    
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "faddp %%st(1), %%st\n\t"
        "fistpl %0"
        : "=m"(i11)
        : "m"(ld8), "m"(ld3)
        : "st", "st(1)", "memory"
    );
    
    /* 4. Another multi-alternative pattern with output in x87 register */
    ld9 = ld1;
    __asm__ volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fsubrp %%st(1), %%st\n\t"
        : "=t"(ld10)
        : "t"(ld9), "m"(ld4)
        : "st(1)"
    );
    
    /* 5. Complex pattern with both input and output alternatives */
    /* This may trigger secondary_in_reload and secondary_out_reload */
    {
        volatile long double ld_temp = ld5;
        volatile int int_temp = i9;
        
        __asm__ volatile (
            "fldt %2\n\t"
            "fildl %1\n\t"
            "faddp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m"(ld11)
            : "rm,t"(int_temp), "m"(ld_temp)
            : "st", "st(1)"
        );
    }
    
    /* 6. Chain operations to increase register pressure */
    for (volatile int loop = 0; loop < 3; loop++) {
        ld12 = ld6 + ld7;
        
        /* Use builtin that requires specific registers */
        {
            uint32_t crc = 0xFFFFFFFF;
            crc = __builtin_ia32_crc32qi(crc, (uint8_t)(i10 + loop));
            i12 = crc;
        }
        
        /* Another x87 operation in the loop */
        __asm__ volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "fmulp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m"(global_results[result_index % 20])
            : "m"(ld12), "m"(ld8)
            : "st", "st(1)"
        );
        
        result_index++;
    }
    
    /* Store results to prevent optimization */
    global_ints[0] = i11;
    global_ints[1] = i12;
    global_results[10] = ld10;
    global_results[11] = ld11;
    
    /* Final mixed operation */
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fdivrp %%st(1), %%st\n\t"
        "fistpl %0"
        : "=m"(i13)
        : "t"(ld10), "m"(ld11)
        : "st", "st(1)"
    );
    
    global_ints[2] = i13;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    for (int i = 0; i < 5; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute and print a checksum to ensure code executes */
    int checksum = 0;
    for (int i = 0; i < 3; i++) {
        checksum += global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
