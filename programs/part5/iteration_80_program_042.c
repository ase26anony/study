/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper_funcs.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>

/* External declarations for helper functions */
extern int __attribute__((noinline)) non_inlineable_func(int x, int y);
extern int __attribute__((const)) pure_func(int x);
extern void init_arrays(void);
extern int compute_checksum(void);

/* Global arrays with potential aliasing */
volatile int g_volatile_seed = 42;
int g_array1[1024];
int g_array2[1024];
short g_short_array[2048];
unsigned g_unsigned_array[512];

/* Struct to create complex memory access patterns */
struct DataStruct {
    int a;
    int b;
    short c;
    unsigned d;
};

struct DataStruct g_struct_array[256];

/* Non-inlineable function to create scheduling boundaries */
int __attribute__((noinline)) non_inlineable_func(int x, int y) {
    /* Complex enough to not be inlined */
    asm volatile ("" : : "r"(x), "r"(y) : "memory");
    return (x * y) ^ (x + y);
}

/* Pure function for loop-invariant removal testing */
int __attribute__((const)) pure_func(int x) {
    return x * x - x + 1;
}

/* Warm-up function to trigger compilation paths */
void warm_up_computation(int iterations) {
    register int i, j;
    int temp = 0;
    
    /* Simple warm-up loop with mixed operations */
    for (i = 0; i < iterations; ++i) {
        temp += i * 2;
        if (i % 7 == 0) {
            temp -= pure_func(i);
        }
        /* Memory barrier to prevent optimization */
        asm volatile ("" ::: "memory");
    }
    
    /* Use result to prevent dead code elimination */
    g_array1[0] = temp;
}

/* Core computation with nested loops for selective scheduling */
int core_computation(int outer_limit, int inner_limit, int threshold) {
    int result = 0;
    unsigned u_result = 0;
    short s_temp = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int loop_carried = i;
        
        /* First inner loop - always executed */
        for (register int j = 0; j < inner_limit; ++j) {
            /* Loop-carried dependency */
            loop_carried = g_array1[j] + loop_carried;
            
            /* Mixed data type operations */
            s_temp = (short)(g_short_array[j] * i);
            u_result += (unsigned)s_temp;
            
            /* Conditional with __builtin_expect */
            if (__builtin_expect((j & 3) == 0, 0)) {
                /* Call non-inlineable function */
                result += non_inlineable_func(i, j);
            }
            
            /* Memory operation with potential aliasing */
            g_array2[j] = g_array1[j] * 2 + result;
            
            /* Optimization barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Second inner loop - conditionally executed */
        if (i > threshold) {
            for (unsigned k = 0; k < (unsigned)(inner_limit / 2); ++k) {
                /* Complex addressing pattern */
                int idx = (i * 31 + k * 17) & 255;
                
                /* Struct member access */
                g_struct_array[idx].a = i;
                g_struct_array[idx].b = k;
                g_struct_array[idx].c = s_temp;
                g_struct_array[idx].d = u_result;
                
                /* Pure function call with loop-variant argument */
                result += pure_func(idx) - pure_func(k);
                
                /* Another memory barrier */
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Third loop with different counter type */
        for (short s = 0; s < 100; s += 3) {
            /* Array access with different stride */
            int idx = (i * s) & 1023;
            g_array1[idx] = g_array2[idx] + s;
            
            /* Conditional branch inside innermost loop */
            if (s % 11 == 0) {
                result += g_unsigned_array[s >> 1];
            }
        }
    }
    
    return result + loop_carried + (int)u_result;
}

/* Initialize data with pseudo-random values */
void init_data(void) {
    /* Simple LCG for pseudo-random values */
    unsigned seed = g_volatile_seed;
    
    for (int i = 0; i < 1024; ++i) {
        seed = seed * 1103515245 + 12345;
        g_array1[i] = (int)(seed & 0x7FFF);
        g_array2[i] = (int)((seed >> 16) & 0x7FFF);
    }
    
    for (int i = 0; i < 2048; ++i) {
        seed = seed * 1103515245 + 12345;
        g_short_array[i] = (short)(seed & 0xFFFF);
    }
    
    for (int i = 0; i < 512; ++i) {
        seed = seed * 1103515245 + 12345;
        g_unsigned_array[i] = seed;
    }
    
    for (int i = 0; i < 256; ++i) {
        seed = seed * 1103515245 + 12345;
        g_struct_array[i].a = (int)seed;
        g_struct_array[i].b = (int)(seed >> 8);
        g_struct_array[i].c = (short)(seed & 0xFFFF);
        g_struct_array[i].d = seed;
    }
}

/* Compute final checksum */
int compute_checksum(void) {
    int checksum = 0;
    
    for (int i = 0; i < 1024; ++i) {
        checksum ^= g_array1[i];
        checksum += g_array2[i];
    }
    
    for (int i = 0; i < 256; ++i) {
        checksum += g_struct_array[i].a;
        checksum ^= g_struct_array[i].b;
        checksum -= g_struct_array[i].c;
        checksum += (int)g_struct_array[i].d;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Use command line args or defaults for variability */
    int outer_limit = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_limit = (argc > 2) ? atoi(argv[2]) : 100;
    int threshold = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Ensure limits are reasonable */
    if (outer_limit <= 0) outer_limit = 50;
    if (inner_limit <= 0) inner_limit = 100;
    if (threshold < 0) threshold = 25;
    
    printf("Starting selective scheduling test...\n");
    printf("Parameters: outer=%d, inner=%d, threshold=%d\n", 
           outer_limit, inner_limit, threshold);
    
    /* Initialize data */
    init_data();
    
    /* Warm-up execution */
    printf("Warm-up phase...\n");
    warm_up_computation(100);
    
    /* Main computation with nested loops */
    printf("Main computation phase...\n");
    int result = core_computation(outer_limit, inner_limit, threshold);
    
    /* Compute final checksum */
    int checksum = compute_checksum();
    
    printf("Result: %d\n", result);
    printf("Checksum: %d\n", checksum);
    printf("Final value: %d\n", result + checksum);
    
    return 0;
}
