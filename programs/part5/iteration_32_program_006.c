/* ifcvt_test.c - Target the uncovered lines in GCC's if-conversion pass */
/* Compile with: gcc -O2 -fdump-rtl-ifcvt -fdump-rtl-all ifcvt_test.c -o ifcvt_test */
/* For ARM: gcc -O2 -march=armv7-a -mtune=cortex-a9 -fdump-rtl-ifcvt ifcvt_test.c -o ifcvt_test_arm */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int threshold = 100;

/* Function to prevent optimization */
static int __attribute__((noinline, noclone)) 
use_value(int x) {
    volatile int sink = x;
    return sink;
}

/* Function with the targeted if-then-else structure */
static int __attribute__((noinline, noclone))
test_if_conversion(int a, int b, int* ptr) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    /* This should generate a non-trivial test_expr in RTL */
    if (global_cond > threshold && *ptr != 0 && a < b) {
        /* 
         * This is the 'then' block (then_bb).
         * The header starts with the block label (implicit).
         * We need to modify the condition expression BEFORE the first
         * real instruction in the block.
         */
        
        /* Generate NOTE/DEBUG_INSN instructions first */
        /* These should appear before then_last_head */
        asm volatile("# DEBUG/NOTE: Start of then block header");
        asm volatile("# Another note in header");
        
        /* CRITICAL: Modify a variable used in the condition expression
         * This instruction should be in the header portion before
         * then_last_head, since it comes after notes but before
         * other "real" instructions */
        global_cond = 50;  /* Modifies test_expr! */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Now add other instructions to create a multi-instruction block */
        result = a * b + global_cond;
        *ptr = result;
        
        /* More operations to ensure block isn't trivial */
        result += use_value(b);
        global_ptr = ptr;
        
        asm volatile("# End of then block operations");
    } else {
        /* else block */
        result = a - b;
        global_cond = 200;
    }
    
    return result;
}

/* Another test case with different condition structure */
static int __attribute__((noinline, noclone))
test_pointer_modification(int x, int y) {
    static int data[10] = {0};
    int* ptr = &data[0];
    int result = 0;
    
    /* Condition using pointer dereference */
    if (*ptr + x > y && global_cond < 500) {
        /* Header with notes/debug */
        asm volatile("# Note: pointer test header");
        
        /* Modify through pointer - affects condition expression */
        *ptr = x * 2;  /* Modifies *ptr used in condition */
        
        asm volatile("" : : : "memory");
        
        /* Rest of block */
        result = *ptr + y;
        for (int i = 0; i < 5; i++) {
            data[i] = result + i;
        }
        global_cond += result;
    } else {
        result = y - x;
        *ptr = result;
    }
    
    return result;
}

/* Test with volatile to force memory operations */
static int __attribute__((noinline, noclone))
test_volatile_cond(int val) {
    volatile int local_cond = val;
    int temp = 0;
    
    if (local_cond > 0 && global_cond != 0) {
        /* Notes in header */
        asm volatile("# Volatile test header note 1");
        asm volatile("# Volatile test header note 2");
        
        /* Modify condition variable */
        local_cond = -1;  /* Affects condition */
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Block body */
        temp = local_cond * 2;
        global_cond = temp;
        temp += use_value(val);
        
        asm volatile("# End volatile block");
    } else {
        temp = val * 3;
        local_cond = temp;
    }
    
    return temp;
}

int main(void) {
    int data = 42;
    int* ptr = &data;
    int results[3] = {0};
    
    /* Initialize globals */
    global_cond = 75;  /* Will be > threshold in some calls */
    threshold = 50;
    
    /* Call test functions multiple times with different parameters
     * to exercise different paths and ensure if-conversion runs */
    
    /* First call: should take then-branch */
    global_cond = 75;  /* > threshold */
    data = 10;         /* *ptr != 0 */
    results[0] = test_if_conversion(5, 10, ptr);
    
    /* Second call: should take else-branch */
    global_cond = 25;  /* < threshold */
    results[1] = test_if_conversion(15, 10, ptr);
    
    /* Third call: test pointer modification */
    results[2] = test_pointer_modification(20, 30);
    
    /* Fourth call: test volatile */
    int vol_result = test_volatile_cond(100);
    
    /* Fifth call: vary parameters more */
    for (int i = 0; i < 5; i++) {
        global_cond = i * 50;
        data = i * 10;
        test_if_conversion(i, i*2, ptr);
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < 3; i++) {
        final_result += results[i];
    }
    final_result += vol_result;
    
    printf("Result: %d (value not important, just to use variables)\n", final_result);
    
    /* Additional test with function pointer to create complex RTL */
    {
        int (*func_ptr)(int, int, int*) = test_if_conversion;
        if (func_ptr != NULL) {
            final_result += func_ptr(1, 2, ptr);
        }
    }
    
    return final_result != 0 ? 0 : 1;
}
