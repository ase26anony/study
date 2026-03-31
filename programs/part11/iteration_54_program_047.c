/* test_hw_doloop.c
 * Test program to cover uncovered lines in GCC's hw-doloop pass
 * Lines 429-436 in hw-doloop.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
static volatile int force_loop = 1;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer */
__attribute__((target("arch=armv7-a")))
void pattern_a_perfect_nesting(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n && force_loop; i++) {
        /* Add conditional to create more basic blocks */
        if (__builtin_expect(i % 2, 0)) {
            /* Empty block to affect bitmap */
        }
        
        /* Inner loop - perfectly nested */
        for (int j = 0; j < m && force_loop; j++) {
            sum += i * j;
            /* Split block with conditional */
            if (__builtin_expect(j % 3, 1)) {
                sum += 1;
            }
        }
        
        /* Another statement in outer loop */
        if (__builtin_expect(i % 5, 0)) {
            sum -= 1;
        }
    }
    
    /* Prevent removal */
    if (sum == 0) abort();
}

/* Pattern B: Partially overlapping loops with shared blocks */
__attribute__((target("arch=armv7-a")))
void pattern_b_partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0, j = 0;
    
    /* Shared setup block */
    int shared = n * 2;
    
    /* First loop */
    while (i < n && force_loop) {
        sum += i;
        /* Shared block between loops */
        if (shared > 0) {
            sum += shared;
            shared--;
        }
        
        /* Conditional goto to create irregular CFG */
        if (__builtin_expect(i % 7, 0)) {
            goto skip_increment;
        }
        i++;
        continue;
        
    skip_increment:
        i += 2;
    }
    
    /* Second loop that partially overlaps */
    j = 0;
    do {
        /* Same shared block access */
        if (shared < 10) {
            sum -= shared;
        }
        
        /* Different block not in first loop */
        for (int k = 0; k < 3 && force_loop; k++) {
            sum += k * j;
        }
        
        j++;
    } while (j < m && force_loop);
    
    if (sum == 0) abort();
}

/* Pattern C: Sequential loops with shared preheader */
__attribute__((target("arch=armv7-a")))
void pattern_c_sequential_shared(int n) {
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n * 3;
    if (setup > 100) {
        setup = 100;
    }
    
    /* First loop */
    for (int i = 0; i < n && force_loop; i++) {
        sum += i + setup;
        /* Early exit creates different block structure */
        if (i > n/2) {
            break;
        }
    }
    
    /* Shared middle block */
    setup /= 2;
    
    /* Second loop - shares some blocks but not all */
    int j = n;
    while (j-- > 0 && force_loop) {
        sum -= j + setup;
        /* Nested mini-loop */
        for (int k = 0; k < 2 && force_loop; k++) {
            sum += k;
        }
    }
    
    if (sum == 0) abort();
}

/* Pattern D: Complex nested structure with multiple levels */
__attribute__((target("arch=armv7-a")))
void pattern_d_complex_nesting(int n) {
    int sum = 0;
    
    /* Level 1 */
    for (int a = 0; a < n && force_loop; a++) {
        /* Level 2 */
        int b = 0;
        while (b < n && force_loop) {
            /* Level 3 - innermost */
            for (int c = 0; c < 5 && force_loop; c++) {
                sum += a * b * c;
                
                /* Conditional break affects bitmap */
                if (c == 3 && __builtin_expect(a % 2, 0)) {
                    break;
                }
            }
            
            /* Another loop at same level as while */
            for (int d = 0; d < 2 && force_loop; d++) {
                sum += d;
                if (__builtin_expect(d == 1, 0)) {
                    goto early_exit;
                }
            }
            
            b++;
            continue;
            
        early_exit:
            b += 2;
        }
        
        /* Loop at same level as outer for */
        int e = 0;
        do {
            sum -= e;
            e++;
        } while (e < 3 && force_loop);
    }
    
    if (sum == 0) abort();
}

/* Pattern E: Loops with function calls containing loops */
__attribute__((target("arch=armv7-a")))
static void helper_with_loop(int *sum, int count) {
    for (int i = 0; i < count && force_loop; i++) {
        *sum += i * 2;
        /* Split block */
        if (__builtin_expect(i % 4, 0)) {
            *sum += 1;
        }
    }
}

__attribute__((target("arch=armv7-a")))
void pattern_e_function_calls(int n) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n && force_loop; i++) {
        sum += i;
        
        /* Call function with loop */
        helper_with_loop(&sum, 5);
        
        /* Another loop after call */
        int j = 0;
        while (j < 3 && force_loop) {
            sum -= j;
            j++;
        }
    }
    
    if (sum == 0) abort();
}

/* Main function to ensure all patterns are called */
int main() {
    /* Use different parameters to create varied loop executions */
    int n = 100;
    int m = 50;
    
    /* Call all patterns */
    pattern_a_perfect_nesting(n, m);
    pattern_b_partial_overlap(n, m);
    pattern_c_sequential_shared(n);
    pattern_d_complex_nesting(n/10);
    pattern_e_function_calls(n);
    
    return 0;
}
