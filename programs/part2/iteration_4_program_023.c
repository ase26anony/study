/* Target: MIPS architecture with delay slots */
/* Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -S -o reorg_test.s reorg_test.c */
/* Or for testing: mips-linux-gnu-gcc -O2 -march=mips32 -fdump-rtl-reorg -fdump-rtl-all reorg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not default */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void delay_slot_pattern(int *result, const int *data, int n) {
    int i;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    int a = 1, b = 2, c = 3, d = 4;
    volatile int barrier = 0; /* Use for memory barriers */
    
    /* Initialize with simple operations to create resource sets */
    temp1 = a + b;
    temp2 = c - d;
    
    for (i = 0; i < n; i++) {
        int val = data[i];
        
        /* Pattern 1: Simple jump to label with eligible follower */
        if (__builtin_expect(val > 100, 0)) {
            /* This should become a simplejump to label1 */
            goto label1;
        }
        
        /* Some intermediate computation using different registers */
        temp3 = temp1 * temp2;
        barrier = temp3; /* Memory barrier */
        
        /* Continue normal path */
        *result += val;
        continue;
        
        /* Target label with eligible instruction */
        label1:
        /* This instruction must be:
           - Non-jump
           - Non-trapping (no division by variable)
           - Not a SEQUENCE
           - Using resources not in &set or &needed
           - Splittable by try_split
        */
        a = b + c;  /* Simple integer add, uses different regs than delay slot would */
        *result += val * 2;
        
        /* Insert memory barrier to constrain scheduling */
        asm volatile("" ::: "memory");
    }
    
    /* Pattern 2: Nested loop with multiple jump-label patterns */
    for (i = 0; i < n; i++) {
        int x = data[i];
        
        /* Create pressure for delay slot filling */
        if (__builtin_expect(x & 1, 1)) {
            goto even_path;
        }
        
        /* Alternate path with different resource usage */
        temp1 = temp2 ^ temp3;  /* Bitwise op, non-trapping */
        goto continue_loop;
        
        even_path:
        /* Candidate for delay slot filling */
        b = c ^ d;  /* Simple bitwise operation */
        temp1 = x * 3;
        
        continue_loop:
        /* Mix integer and bit operations to diversify resource patterns */
        *result += temp1;
        
        /* Another jump pattern in the same loop */
        if (__builtin_expect((x % 7) == 0, 0)) {
            goto special_case;
        }
        
        /* Normal case computation */
        c = d + 1;
        continue;
        
        special_case:
        /* Another eligible candidate instruction */
        d = a & 0xFF;  /* Bitwise AND, non-trapping */
        *result += x * 5;
    }
}

/* Pattern 3: Complex control flow with multiple labels */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void multi_label_pattern(int *arr1, int *arr2, int size) {
    int i, j;
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int reg_a = 1, reg_b = 2, reg_c = 3, reg_d = 4;
    int reg_e = 5, reg_f = 6, reg_g = 7, reg_h = 8;
    
    for (i = 0; i < size; i++) {
        /* Create multiple basic blocks with jumps */
        if (arr1[i] < 0) {
            goto handle_negative;
        }
        
        if (arr1[i] > 1000) {
            goto handle_large;
        }
        
        /* Medium value processing */
        reg_a = reg_b + reg_c;
        arr2[i] = arr1[i] * 2;
        goto next_iteration;
        
        handle_negative:
        /* Candidate instruction for delay slot */
        reg_d = reg_e - reg_f;  /* Simple subtraction */
        arr2[i] = -arr1[i];
        goto next_iteration;
        
        handle_large:
        /* Another candidate */
        reg_g = reg_h & 0x7F;  /* Bitwise AND with constant */
        arr2[i] = arr1[i] / 4;  /* Division by constant is safe */
        
        next_iteration:
        /* Force register usage to prevent optimization */
        r1 += reg_a; r2 += reg_b; r3 += reg_c; r4 += reg_d;
        
        /* Inner loop to increase scheduling complexity */
        for (j = 0; j < 4; j++) {
            /* Small inner loop with conditional */
            if (__builtin_expect((i + j) % 3 == 0, 0)) {
                goto inner_label;
            }
            
            reg_e = reg_f ^ reg_g;
            continue;
            
            inner_label:
            /* Inner loop candidate */
            reg_h = reg_a | reg_b;  /* Bitwise OR */
            r1 += j;
        }
    }
    
    /* Use results to prevent dead code elimination */
    arr2[0] = r1 + r2 + r3 + r4;
}

/* Pattern 4: Switch-like structure with goto labels */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
int switch_like_pattern(int x) {
    int result = 0;
    int t1 = 10, t2 = 20, t3 = 30, t4 = 40;
    
    /* Simulate switch with direct jumps */
    if (x == 1) goto case1;
    if (x == 2) goto case2;
    if (x == 3) goto case3;
    goto default_case;
    
    case1:
    /* Eligible instruction for delay slot */
    t1 = t2 + t3;  /* Simple add */
    result = x * 10;
    goto end;
    
    case2:
    t2 = t3 - t4;  /* Simple subtract */
    result = x * 20;
    goto end;
    
    case3:
    t3 = t1 ^ t2;  /* Bitwise XOR */
    result = x * 30;
    goto end;
    
    default_case:
    t4 = t1 & t2;  /* Bitwise AND */
    result = x * 40;
    
    end:
    /* Use all temporaries to keep them alive */
    return result + t1 + t2 + t3 + t4;
}

/* Main function that exercises all patterns */
int main() {
    const int SIZE = 1000;
    int *data = (int*)malloc(SIZE * sizeof(int));
    int *result1 = (int*)malloc(SIZE * sizeof(int));
    int final_result = 0;
    
    /* Initialize with pattern that creates various branch conditions */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 37) % 123;  /* Creates mix of values */
    }
    
    /* Exercise pattern 1 */
    delay_slot_pattern(&final_result, data, SIZE);
    
    /* Exercise pattern 2 - reuse data array */
    multi_label_pattern(data, result1, SIZE);
    
    /* Exercise pattern 3 */
    for (int i = 0; i < 100; i++) {
        final_result += switch_like_pattern(i % 5);
    }
    
    /* Use results to prevent optimization */
    for (int i = 0; i < SIZE; i++) {
        final_result += result1[i];
    }
    
    printf("Final result: %d\n", final_result);
    
    free(data);
    free(result1);
    
    return 0;
}
