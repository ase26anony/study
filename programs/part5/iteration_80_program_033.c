/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>

/* External declarations for multi-file compilation stress */
extern int helper_pure(int a, int b) __attribute__((const));
extern void helper_noinline(int *arr, int idx) __attribute__((noinline));
extern volatile int g_volatile_seed;

/* Global arrays for memory operations with potential aliasing */
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
    int arr[8];
};

struct DataStruct g_struct_array[128];

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
int compute_value(int x, int y) {
    /* Complex enough to not be inlined */
    asm volatile("" : : : "memory");
    return (x * y) + (x >> 3) - (y << 2);
}

/* Pure function for loop-variant calls */
__attribute__((const))
int pure_transform(int val) {
    return (val * 13 + 7) & 0xFF;
}

/* Function with loop-carried dependency */
int process_loops(int outer_limit, int inner_limit, int threshold) {
    int acc = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    int temp;
    unsigned u_acc = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        /* Conditional based on outer loop index */
        if (__builtin_expect(i > threshold, 0)) {
            /* Inner loop 1: executed conditionally */
            for (short s = 0; s < (short)(inner_limit / 2); ++s) {
                /* Memory operation with potential aliasing */
                g_array1[i * 16 + s] = g_array2[s * 8] + i;
                
                /* Loop-carried dependency */
                acc = g_array1[i * 16 + s] + acc;
                
                /* Function call with loop-variant arguments */
                temp = pure_transform(s + i);
                
                /* Mixed data type operations */
                u_acc += (unsigned)temp * (unsigned)s;
                
                /* Optimization barrier */
                asm volatile("" ::: "memory");
            }
        } else {
            /* Different computation path */
            for (unsigned j = 0; j < (unsigned)(inner_limit); ++j) {
                /* Access struct array */
                g_struct_array[j % 128].a = i;
                g_struct_array[j % 128].b = j;
                
                /* Complex arithmetic with different types */
                int val = compute_value(i, j);
                g_short_array[j * 2] = (short)(val & 0xFFFF);
                g_short_array[j * 2 + 1] = (short)((val >> 16) & 0xFFFF);
                
                /* Register variable usage */
                reg_acc += val;
                
                /* Conditional branch inside inner loop */
                if (__builtin_expect((j & 0x3F) == 0, 0)) {
                    /* Non-inlineable function call */
                    helper_noinline(g_array1, j);
                }
                
                /* Another optimization barrier */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Nested loop with different counter type */
        for (int k = 0; k < 8; ++k) {
            /* Multiple uses of same variable */
            int idx = i * 8 + k;
            g_array2[idx % 1024] = pure_transform(idx);
            
            /* Complex expression with multiple operations */
            int t1 = g_array1[idx % 1024];
            int t2 = g_array2[(idx + 1) % 1024];
            int t3 = helper_pure(t1, t2);
            
            /* Variable with different scope */
            {
                int local_var = t3 * k;
                acc += local_var;
                reg_acc ^= local_var;
            }
        }
    }
    
    /* Combine accumulators */
    return acc + (int)u_acc + reg_acc;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline))
int warmup_computation(int iterations) {
    int result = 0;
    volatile int vol_counter = iterations;
    
    /* Simple warm-up loop */
    for (int i = 0; i < vol_counter; ++i) {
        result += pure_transform(i);
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Initialize data with pseudo-random values using LCG */
void initialize_data(void) {
    unsigned seed = 12345;  /* Fixed seed for reproducibility */
    
    for (int i = 0; i < 1024; ++i) {
        seed = seed * 1103515245 + 12345;
        g_array1[i] = (int)(seed >> 16) & 0x7FFF;
        g_array2[i] = (int)(seed >> 8) & 0xFF;
    }
    
    for (int i = 0; i < 2048; ++i) {
        seed = seed * 1103515245 + 12345;
        g_short_array[i] = (short)(seed & 0xFFFF);
    }
    
    for (int i = 0; i < 128; ++i) {
        for (int j = 0; j < 8; ++j) {
            seed = seed * 1103515245 + 12345;
            g_struct_array[i].arr[j] = (int)(seed & 0xFF);
        }
        g_struct_array[i].a = i;
        g_struct_array[i].b = i * 2;
        g_struct_array[i].c = (short)i;
        g_struct_array[i].d = (unsigned)i * 3;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments for variability */
    int outer_limit = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_limit = (argc > 2) ? atoi(argv[2]) : 100;
    int threshold = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Make limits volatile to prevent optimization */
    volatile int vol_outer = outer_limit;
    volatile int vol_inner = inner_limit;
    volatile int vol_thresh = threshold;
    
    printf("Starting selective scheduling test...\n");
    printf("Parameters: outer=%d, inner=%d, threshold=%d\n", 
           vol_outer, vol_inner, vol_thresh);
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up computation */
    printf("Warm-up computation: %d\n", warmup_computation(100));
    
    /* Main computation with nested loops */
    int result = process_loops(vol_outer, vol_inner, vol_thresh);
    
    /* Additional computation with different patterns */
    int checksum = result;
    
    /* Another loop pattern with different characteristics */
    for (int i = 0; i < vol_outer / 2; ++i) {
        for (unsigned j = 0; j < (unsigned)vol_inner; j += 4) {
            /* Vector-like operation */
            int sum = 0;
            for (int k = 0; k < 4; ++k) {
                sum += g_array1[(i * 4 + k) % 1024];
            }
            checksum ^= sum;
            
            /* Conditional with unpredictable pattern */
            if ((i * j) & 0x1F) {
                checksum += helper_pure(i, j);
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
