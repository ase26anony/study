/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>

/* External declarations for multi-file compilation stress */
extern int helper_pure(int a, int b) __attribute__((const));
extern void helper_noinline(int *arr, int idx) __attribute__((noinline));
extern void helper_memory_barrier(void) __attribute__((noinline));

/* Global arrays for memory operations with potential aliasing */
volatile int g_volatile_seed = 42;
int g_array1[1024];
int g_array2[1024];
short g_short_array[2048];
unsigned g_unsigned_array[512];

/* Struct for complex memory access patterns */
struct DataStruct {
    int a;
    int b;
    short c;
    unsigned d;
};

struct DataStruct g_struct_array[256];

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
int compute_checksum(int a, int b, int c) {
    /* Complex enough to not be inlined */
    int result = a ^ b;
    result = (result << 3) | (result >> 29);
    result += c * 7;
    result ^= 0x5A5A5A5A;
    return result;
}

/* Pure function for loop-variant calls */
__attribute__((const))
int pure_transform(int x, int y) {
    return (x * 3 + y * 7) & 0xFF;
}

/* Function with mixed operations to create interesting RTL */
void process_loops(int outer_limit, int inner_limit, int *result) {
    int i, j, k;
    unsigned u;
    short s;
    register int reg_acc1 __asm__("r12") = 0;  /* Hint at register allocation */
    int reg_acc2 = 0;
    int local_array[64];
    
    /* Initialize local array with varying values */
    for (k = 0; k < 64; ++k) {
        local_array[k] = k * 3 + g_volatile_seed;
    }
    
    /* Nested loops with varying trip counts and data types */
    for (i = 0; i < outer_limit; ++i) {
        int loop_carried = i * 17;
        unsigned u_i = (unsigned)i;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > (outer_limit / 3), 0)) {
            /* This inner loop only executes conditionally */
            for (j = 0; j < inner_limit / 2; ++j) {
                /* Mixed arithmetic operations */
                int temp = g_array1[i] + g_array2[j];
                temp = pure_transform(temp, j);
                
                /* Memory operation with potential aliasing */
                g_short_array[i * 2 + j] = (short)(temp & 0xFFFF);
                
                /* Loop-carried dependency */
                reg_acc1 = reg_acc1 + temp;
                
                /* Optimization barrier */
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Main inner loop with complex operations */
        for (j = 0; j < inner_limit; ++j) {
            /* Use different data types */
            s = (short)j;
            u = (unsigned)(i * j);
            
            /* Memory access to global arrays */
            int val1 = g_array1[(i + j) % 1024];
            int val2 = g_array2[(i * 3 + j) % 1024];
            
            /* Complex arithmetic with mixed types */
            int combined = val1 * 3 + val2 * 7;
            combined += (int)s * 5;
            combined ^= (int)u;
            
            /* Function call with loop-variant arguments */
            combined = compute_checksum(combined, i, j);
            
            /* Access struct array */
            g_struct_array[(i + j) % 256].a = combined;
            g_struct_array[(i + j) % 256].b = i;
            
            /* Multiple uses of same variable */
            reg_acc2 = reg_acc2 + combined;
            loop_carried = loop_carried * 3 + combined;
            
            /* Conditional branch inside innermost loop */
            if (__builtin_expect((i * j) % 13 == 0, 0)) {
                /* Call non-inlineable helper */
                helper_noinline(local_array, j % 64);
                
                /* Additional memory operation */
                g_unsigned_array[u % 512] = u;
            }
            
            /* Another optimization barrier */
            asm volatile ("" ::: "memory");
            
            /* Pure function call */
            int transformed = pure_transform(combined, loop_carried);
            reg_acc1 += transformed;
            
            /* Access different array with short type */
            g_short_array[(i * 4 + j * 3) % 2048] = (short)(transformed & 0xFFFF);
        }
        
        /* Third level of nesting for some iterations */
        if (i % 7 == 0) {
            for (k = 0; k < 8; ++k) {
                /* Different operation mix */
                int temp = g_struct_array[(i + k) % 256].a;
                temp = (temp << k) | (temp >> (32 - k));
                reg_acc2 ^= temp;
                
                /* Call helper from another file */
                temp = helper_pure(temp, k);
                g_array1[(i + k) % 1024] = temp;
            }
        }
    }
    
    /* Combine results */
    *result = reg_acc1 ^ reg_acc2;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline))
void warm_up_computation(void) {
    int dummy_result;
    volatile int warm_limit = 10;
    
    /* Simple warm-up loop */
    for (int i = 0; i < warm_limit; ++i) {
        dummy_result = pure_transform(i, i * 2);
        g_array1[i % 1024] = dummy_result;
    }
    
    /* Memory barrier helper */
    helper_memory_barrier();
}

/* Initialize data with pseudo-random values using LCG */
void initialize_data(void) {
    unsigned lcg = g_volatile_seed;
    
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_array1[i] = (int)(lcg % 1000);
        g_array2[i] = (int)((lcg >> 16) % 1000);
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_short_array[i] = (short)(lcg % 32768);
    }
    
    for (int i = 0; i < 256; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_struct_array[i].a = (int)(lcg % 10000);
        g_struct_array[i].b = (int)((lcg >> 8) % 10000);
        g_struct_array[i].c = (short)(lcg % 256);
        g_struct_array[i].d = (unsigned)lcg;
    }
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments for loop bounds to prevent optimization */
    int outer_limit = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_limit = (argc > 2) ? atoi(argv[2]) : 100;
    
    /* Ensure non-zero limits */
    if (outer_limit <= 0) outer_limit = 50;
    if (inner_limit <= 0) inner_limit = 100;
    
    printf("Starting selective scheduling test with limits: %d, %d\n", 
           outer_limit, inner_limit);
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up to trigger different compilation paths */
    printf("Performing warm-up...\n");
    warm_up_computation();
    
    /* Main computation with nested loops */
    printf("Running main computation...\n");
    process_loops(outer_limit, inner_limit, &result);
    
    /* Additional computation to ensure all code paths are used */
    int temp_result = 0;
    for (int i = 0; i < outer_limit / 4; ++i) {
        for (int j = 0; j < inner_limit / 4; ++j) {
            temp_result += pure_transform(g_array1[i], g_array2[j]);
            if ((i + j) % 11 == 0) {
                temp_result ^= compute_checksum(i, j, temp_result);
            }
        }
    }
    
    result ^= temp_result;
    
    /* Print verifiable result */
    printf("Computation checksum: %d (0x%08x)\n", result, result);
    
    return 0;
}
