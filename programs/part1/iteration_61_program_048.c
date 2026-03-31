#include <stdio.h>

extern void bar(void);  // External function to prevent optimization

// Function to test loops with different counter types and contexts
void test_loops(int param_counter) {
    int total = 0;
    
    // Pattern 1: Basic signed int counter - should match the pattern
    printf("Loop 1 (signed int): ");
    int counter1 = 10;
    do {
        total += 1;
        bar();  // External call prevents dead code elimination
    } while (--counter1 > 0);
    printf("counted %d iterations\n", 10 - counter1);
    
    // Pattern 2: Unsigned int counter with != 0 comparison
    printf("Loop 2 (unsigned int): ");
    unsigned int u_counter = 5;
    do {
        total += 2;
        bar();
    } while (--u_counter != 0);
    printf("counted %u iterations\n", 5 - u_counter);
    
    // Pattern 3: register-qualified counter
    printf("Loop 3 (register int): ");
    register int reg_counter = 3;
    do {
        total += 3;
        bar();
    } while (--reg_counter > 0);
    printf("counted %d iterations\n", 3 - reg_counter);
    
    // Pattern 4: short type counter
    printf("Loop 4 (short): ");
    short short_counter = 7;
    do {
        total += 4;
        bar();
    } while (--short_counter > 0);
    printf("counted %d iterations\n", 7 - short_counter);
    
    // Pattern 5: char type counter
    printf("Loop 5 (char): ");
    char char_counter = 4;
    do {
        total += 5;
        bar();
    } while (--char_counter > 0);
    printf("counted %d iterations\n", 4 - char_counter);
    
    // Pattern 6: Counter as function parameter
    printf("Loop 6 (parameter): ");
    int param_copy = param_counter;
    do {
        total += 6;
        bar();
    } while (--param_copy > 0);
    printf("counted %d iterations\n", param_counter - param_copy);
    
    // Pattern 7: Loop inside if statement
    printf("Loop 7 (in if): ");
    if (total > 0) {
        int if_counter = 2;
        do {
            total += 7;
            bar();
        } while (--if_counter > 0);
        printf("counted %d iterations\n", 2 - if_counter);
    }
    
    // Pattern 8: Counter starting at 1 (edge case)
    printf("Loop 8 (start=1): ");
    int edge_counter = 1;
    do {
        total += 8;
        bar();
    } while (--edge_counter > 0);
    printf("counted %d iterations\n", 1 - edge_counter);
    
    // Pattern 9: Alternative syntax with assignment in condition
    printf("Loop 9 (assignment): ");
    int assign_counter = 6;
    do {
        total += 9;
        bar();
    } while ((assign_counter -= 1) != 0);
    printf("counted %d iterations\n", 6 - assign_counter);
    
    // Pattern 10: Loop with pointer in body (different body complexity)
    printf("Loop 10 (pointer body): ");
    int ptr_counter = 3;
    int dummy = 0;
    int *ptr = &dummy;
    do {
        *ptr = total;  // Simple pointer store
        bar();
    } while (--ptr_counter > 0);
    printf("counted %d iterations\n", 3 - ptr_counter);
    
    // NON-MATCHING PATTERNS (should NOT trigger the uncovered code)
    
    // Pattern A: Post-increment (should fail GEN_INT(-1) check)
    printf("Loop A (post-inc, non-matching): ");
    int post_counter = 3;
    do {
        total += 100;
        bar();
    } while (post_counter++ < 5);
    printf("counted to %d\n", post_counter);
    
    // Pattern B: Compare against non-zero (should fail const0_rtx check)
    printf("Loop B (compare >5, non-matching): ");
    int cmp_counter = 10;
    do {
        total += 200;
        bar();
    } while (--cmp_counter > 5);
    printf("counted %d iterations\n", 10 - cmp_counter);
    
    // Pattern C: volatile counter (likely won't match due to memory ops)
    printf("Loop C (volatile, non-matching): ");
    volatile int vol_counter = 3;
    do {
        total += 300;
        bar();
    } while (--vol_counter > 0);
    printf("counted volatile iterations\n");
    
    printf("Total accumulated: %d\n", total);
}

// Additional test function with different context
void test_nested_context() {
    int outer = 2;
    
    while (outer-- > 0) {
        // Pattern 11: Loop in nested context
        printf("Loop 11 (nested context): ");
        int inner_counter = 4;
        int local_sum = 0;
        do {
            local_sum += inner_counter;
            bar();
        } while (--inner_counter > 0);
        printf("local sum = %d\n", local_sum);
        
        // Pattern 12: Loop followed by other statements
        printf("Loop 12 (with follow-up): ");
        int follow_counter = 3;
        do {
            bar();
        } while (--follow_counter > 0);
        int after_loop = follow_counter * 10;  // Additional statement
        printf("after = %d\n", after_loop);
    }
}

int main() {
    printf("=== Testing doloop pattern matching ===\n");
    
    // Call test functions with different parameters
    test_loops(4);  // Test with parameter = 4
    
    printf("\n=== Testing nested contexts ===\n");
    test_nested_context();
    
    // Final accumulation to ensure compiler keeps everything
    int final_check = 0;
    int final_counter = 2;
    do {
        final_check += 999;
        bar();
    } while (--final_counter > 0);
    
    printf("\nFinal check value: %d\n", final_check);
    printf("All loops completed.\n");
    
    return 0;
}

// Dummy implementation of bar() to allow linking
void bar(void) {
    // Empty but non-const, non-pure function
    static int call_count = 0;
    call_count++;
}
