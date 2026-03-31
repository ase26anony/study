/* Target: MIPS architecture with delay slots */
#ifndef __mips__
#error "This code is designed for MIPS architecture with delay slots. Compile with -march=mips"
#endif

#include <stdio.h>
#include <stdlib.h>

/* Force delay slot usage and inhibit optimizations */
#define NO_INLINE __attribute__((noinline))
#define VOLATILE_BARRIER asm volatile("" ::: "memory")

/* Resource separation: Use distinct register sets for different operations */
static int global_counter = 0;

/* Function with complex control flow to stress the scheduler */
NO_INLINE static int process_data(int *data_a, int *data_b, int size) {
    int result = 0;
    int temp1, temp2, temp3, temp4;
    volatile int dummy = 0; /* Prevent dead code elimination */
    
    /* Initialize distinct variables for resource separation */
    int reg_set1_a = 0, reg_set1_b = 0;
    int reg_set2_a = 0, reg_set2_b = 0;
    int reg_set3_a = 0, reg_set3_b = 0;
    
    for (int i = 0; i < size; i++) {
        /* Create multiple basic blocks with goto patterns */
        if (__builtin_expect((data_a[i] & 1) == 0, 0)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            goto label_arithmetic_1;
            
            /* This should never be reached directly */
            dummy++;
        }
        
        if (__builtin_expect(data_b[i] < 0, 1)) {
            /* Pattern 2: Another jump pattern */
            goto label_arithmetic_2;
            
            dummy += 2;
        }
        
        /* Main computation path */
        reg_set1_a = data_a[i];
        reg_set1_b = data_b[i];
        reg_set1_a = reg_set1_a + reg_set1_b;  /* Simple non-trapping arithmetic */
        result += reg_set1_a;
        
        /* Memory barrier to constrain scheduling */
        VOLATILE_BARRIER;
        
        continue;  /* Skip the label blocks when not jumping */
        
    /* Label blocks with simple, eligible instructions */
    label_arithmetic_1:
        /* Simple arithmetic that doesn't trap and uses different registers */
        reg_set2_a = data_a[i] ^ 0x55AA55AA;  /* Bitwise operation - no traps */
        reg_set2_b = data_b[i] & 0x00FF00FF;
        reg_set2_a = reg_set2_a + reg_set2_b;  /* Eligible for delay slot */
        result += reg_set2_a;
        
        /* Force a small basic block */
        if (reg_set2_a > 1000) {
            goto small_block_1;
        }
        continue;
        
    small_block_1:
        dummy++;
        continue;
        
    label_arithmetic_2:
        /* Another simple arithmetic pattern with different registers */
        reg_set3_a = data_a[i] * 3;  /* Multiplication by constant - no overflow trap */
        reg_set3_b = data_b[i] >> 2; /* Shift operation - safe */
        reg_set3_a = reg_set3_a - reg_set3_b;  /* Eligible for delay slot */
        result += reg_set3_a;
        
        /* Nested conditional to create more scheduling contexts */
        if (__builtin_expect(reg_set3_a != 0, 1)) {
            goto small_block_2;
        }
        continue;
        
    small_block_2:
        dummy += 2;
        continue;
    }
    
    return result;
}

/* Secondary function with different jump patterns */
NO_INLINE static int alternate_path(int *data, int size) {
    int sum = 0;
    int local_a, local_b, local_c;
    
    for (int i = 0; i < size; i += 2) {
        /* Create jump-to-label pattern in loop */
        if (__builtin_expect(data[i] > data[i + 1], 0)) {
            goto compute_diff;
        }
        
        /* Normal path */
        local_a = data[i];
        local_b = data[i + 1];
        local_c = local_a | local_b;  /* Bitwise OR - safe */
        sum += local_c;
        
        VOLATILE_BARRIER;
        continue;
        
    compute_diff:
        /* Instruction after label: simple subtraction */
        local_a = data[i];
        local_b = data[i + 1];
        local_c = local_a - local_b;  /* Eligible for delay slot */
        sum += local_c;
        
        /* Prevent tail merging */
        if (local_c < 0) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function with mixed operations to diversify resource usage */
NO_INLINE static void mixed_operations(int *arr, int n) {
    float ftemp1 = 0.0f, ftemp2 = 0.0f;
    int itemp1, itemp2;
    
    for (int i = 0; i < n; i++) {
        /* Integer block */
        if (__builtin_expect(arr[i] % 7 == 0, 0)) {
            goto int_op_block;
        }
        
        /* Floating-point block (different resource usage) */
        ftemp1 = (float)arr[i];
        ftemp2 = ftemp1 * 1.5f;
        arr[i] = (int)ftemp2;
        continue;
        
    int_op_block:
        /* Simple integer operation after label */
        itemp1 = arr[i];
        itemp2 = itemp1 << 2;  /* Shift left - safe operation */
        arr[i] = itemp2;
        
        /* Small basic block to prevent sequence formation */
        if (itemp2 > 100) {
            arr[i] = 100;
        }
    }
}

int main(void) {
    const int SIZE = 256;
    int *data_a = malloc(SIZE * sizeof(int));
    int *data_b = malloc(SIZE * sizeof(int));
    
    if (!data_a || !data_b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that creates branch diversity */
    for (int i = 0; i < SIZE; i++) {
        data_a[i] = (i * 37) & 0xFF;
        data_b[i] = (i * 73) & 0xFF;
        
        /* Create some negative values for signed comparisons */
        if (i % 5 == 0) {
            data_b[i] = -data_b[i];
        }
    }
    
    /* Execute functions with jump patterns */
    int result1 = process_data(data_a, data_b, SIZE);
    int result2 = alternate_path(data_a, SIZE);
    
    mixed_operations(data_b, SIZE);
    
    /* Compute final result to prevent elimination */
    int final_result = result1 + result2;
    for (int i = 0; i < SIZE; i++) {
        final_result += data_b[i];
    }
    
    printf("Result: %d\n", final_result);
    printf("Global counter: %d\n", global_counter);
    
    free(data_a);
    free(data_b);
    
    return 0;
}
