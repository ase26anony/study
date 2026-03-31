/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-sched */
/* This program creates patterns to trigger delay slot filling logic */
/* Specifically targeting lines 2135-2149 in reorg.cc */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile to prevent optimization of critical sections */
static volatile int memory_barrier;

/* Resource separation: Use distinct register sets for different operations */
MIPS_TARGET
void delay_slot_test(int *result, const int *data, int size) {
    /* Separate variable sets to avoid resource conflicts */
    int set_a = 0, set_b = 0, set_c = 0;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    int counter = 0;
    
    /* Initialize with non-zero values to avoid constant propagation */
    set_a = data[0] ^ 1;
    set_b = data[1] ^ 2;
    set_c = data[2] ^ 3;
    
    /* Complex loop to create scheduling pressure */
    for (int i = 0; i < size; i++) {
        /* Varying trip count to create unpredictable control flow */
        int inner_limit = (i % 8) + 1;
        
        for (int j = 0; j < inner_limit; j++) {
            /* Branch prediction hints */
            if (__builtin_expect((i ^ j) & 1, 0)) {
                /* Path 1: Jump to label with eligible follower */
                /* This creates a simplejump to a label */
                goto target_label_1;
                
                /* Unreachable code - but creates the jump pattern */
                temp1 = set_a + set_b; /* This won't execute */
                
            target_label_1:
                /* ELIGIBLE FOLLOWER: Simple, non-trapping arithmetic */
                /* This instruction should be candidate for delay slot */
                /* Uses different resources than the jump's context */
                temp3 = set_c + 1; /* Simple add, no trap possible */
                
                /* Continue with more operations */
                set_a = temp3 ^ set_b;
                
            } else {
                /* Path 2: Another jump pattern with different resources */
                if (__builtin_expect((i + j) & 2, 1)) {
                    goto target_label_2;
                    
                target_label_2:
                    /* Another eligible follower */
                    temp2 = set_b - set_a; /* Simple subtract */
                    
                    /* Mix operations to diversify resource usage */
                    set_c = temp2 * 3; /* Multiplication is safe for integers */
                }
            }
            
            /* Insert memory barrier to constrain scheduling */
            if ((i + j) % 16 == 0) {
                __asm__ volatile ("" ::: "memory");
            }
            
            /* More complex control flow with nested conditions */
            switch (counter % 4) {
                case 0:
                    if (set_a > 0) {
                        goto target_label_3;
                        
                    target_label_3:
                        /* Eligible: bitwise operation, no traps */
                        temp1 = set_a & 0xFF;
                        set_b = temp1 | 0x100;
                    }
                    break;
                    
                case 1:
                    /* Force another jump pattern */
                    if (set_b != 0) {
                        goto target_label_4;
                        
                    target_label_4:
                        /* Eligible: shift operation */
                        temp2 = set_c << 2;
                        set_a = temp2 >> 1;
                    }
                    break;
                    
                default:
                    /* Simple arithmetic to maintain variable liveness */
                    set_c = (set_c + 1) & 0xFFFF;
            }
            
            counter++;
            
            /* Prevent loop unrolling completely */
            if (counter % 8 == 0) {
                memory_barrier = set_a;
            }
        }
        
        /* Cross-iteration dependencies to prevent reordering */
        set_a ^= data[i % size];
        set_b += (i & 1) ? 1 : -1;
        
        /* Another jump pattern in the outer loop */
        if (__builtin_expect((set_c & 0x80) != 0, 0)) {
            goto target_label_5;
            
        target_label_5:
            /* Eligible: increment operation */
            temp3 = set_a + 2;
            set_c = temp3 - 1;
        }
    }
    
    /* Store results to prevent dead code elimination */
    result[0] = set_a;
    result[1] = set_b;
    result[2] = set_c;
    result[3] = counter;
}

/* Secondary function with different patterns */
MIPS_TARGET
void nested_jump_patterns(int *output, int iterations) {
    int a = 1, b = 2, c = 3, d = 4;
    int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Jump chain */
        if (i % 3 == 0) {
            goto chain_label_1;
            
        chain_label_1:
            /* Candidate for delay slot */
            a = b + c;  /* Simple, non-trapping */
            
            goto chain_label_2;
            
        chain_label_2:
            b = c - a;  /* Another candidate */
            
            /* Small basic block */
            c = a ^ b;
        }
        
        /* Pattern 2: Jump with immediate follower using different regs */
        if (i % 5 == 0) {
            int t1 = d, t2 = acc;
            
            if (t1 > t2) {
                goto diff_reg_label;
                
            diff_reg_label:
                /* Uses completely different temps */
                int t3 = t1 * 2;  /* Safe multiplication */
                t2 = t3 + 1;
                acc = t2;
            }
            
            d = t1 + 1;
        }
        
        /* Pattern 3: Avoid SEQUENCE formation */
        /* Keep operations atomic and simple */
        if (i % 7 == 0) {
            goto no_sequence_label;
            
        no_sequence_label:
            /* Single, simple operation - won't become SEQUENCE */
            a = (a << 1) | 1;
        }
        
        /* Mix with floating point in different paths */
        if (i % 11 == 0) {
            float fa = (float)a;
            float fb = (float)b;
            /* Different execution unit usage */
            output[i % 16] = (int)(fa * fb);
        }
        
        /* Resource separation: Use 'd' here, not in jump followers */
        d = (d * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Memory op in separate path to affect scheduling */
        if (i % 13 == 0) {
            output[0] = d;
        }
    }
    
    /* Final accumulation */
    output[15] = a + b + c + d + acc;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    int *data = (int*)malloc(SIZE * sizeof(int));
    int *result = (int*)malloc(4 * sizeof(int));
    int *output = (int*)malloc(16 * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Run the tests multiple times */
    for (int run = 0; run < 3; run++) {
        delay_slot_test(result, data, SIZE);
        nested_jump_patterns(output, 100);
        
        /* Use results to prevent optimization */
        printf("Run %d: result = [%d, %d, %d, %d], output[15] = %d\n",
               run, result[0], result[1], result[2], result[3], output[15]);
    }
    
    free(data);
    free(result);
    free(output);
    
    return 0;
}
