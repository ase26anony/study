/* ifcvt_coverage.c
 * Target: Trigger uncovered lines 577-583 in ifcvt.cc
 * Compile with: gcc -O2 -fdump-rtl-ifcvt -S ifcvt_coverage.c
 * For ARM: gcc -O2 -march=armv7-a -mtune=cortex-a9 -fdump-rtl-ifcvt -S ifcvt_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int threshold = 100;

/* Function to generate NOTE/DEBUG_INSN instructions in the header */
static void __attribute__((noinline, noclone)) 
generate_notes(void) {
    /* These asm statements generate NOTE instructions */
    asm volatile("# HEADER NOTE 1");
    asm volatile("# HEADER NOTE 2");
    asm volatile("# HEADER NOTE 3");
}

/* Function with attribute to prevent optimization */
static int __attribute__((noinline, noclone))
target_function(int x, int y) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    /* This should generate a non-trivial test_expr in RTL */
    if (global_cond > threshold && *global_ptr != 0) {
        /* 
         * CRITICAL: Modify the condition variable BEFORE any real instruction
         * This modification happens in the block header (after labels/notes)
         */
        generate_notes();  /* Generates NOTE instructions in header */
        
        /* This modifies global_cond which is part of the condition */
        global_cond = x + y;  /* MODIFICATION IN HEADER - should trigger modified_in_p */
        
        /* Additional statements to create a proper basic block */
        result = x * y;
        global_ptr = &result;
        
        /* More operations to ensure block has multiple instructions */
        for (int i = 0; i < 3; i++) {
            result += i;
        }
        
        /* Store to volatile to prevent dead code elimination */
        asm volatile("" : : "r"(result) : "memory");
    } else {
        result = x - y;
    }
    
    return result;
}

/* Another test case with different condition structure */
static int __attribute__((noinline, noclone))
target_function2(void) {
    static int counter = 0;
    int* ptr = &counter;
    int result = 0;
    
    /* Condition with pointer dereference */
    if (*ptr < 50 && global_cond > 0) {
        /* Generate notes in header */
        asm volatile("# Function2 note 1");
        asm volatile("# Function2 note 2");
        
        /* Modify through pointer - affects condition */
        *ptr = 100;  /* MODIFICATION IN HEADER */
        
        /* Additional code */
        result = global_cond * 2;
        global_cond = result / 3;
        
        asm volatile("" : : "r"(result) : "memory");
    } else {
        result = -1;
        counter++;
    }
    
    return result;
}

/* Test with compound condition where one part is modified */
static int __attribute__((noinline, noclone))
target_function3(int a, int b, int c) {
    volatile int cond1 = a;
    int cond2 = b;
    int result = 0;
    
    /* Compound condition with local variables */
    if (cond1 > 10 && cond2 < 20) {
        /* Notes in header */
        asm volatile("# Note: entering then block");
        
        /* Modify cond2 which is used in condition */
        cond2 = 30;  /* MODIFICATION IN HEADER */
        
        /* Body of then block */
        result = a + b + c;
        for (int i = 0; i < 5; i++) {
            result += i * cond2;
        }
        
        /* Ensure side effect */
        global_cond = result;
    } else {
        result = a - b - c;
    }
    
    return result;
}

/* Main function to exercise different paths */
int main(void) {
    int array[10];
    int sum = 0;
    
    /* Initialize global pointer */
    global_ptr = &array[0];
    *global_ptr = 1;  /* Ensure non-zero for condition */
    
    /* Test 1: Take the then-branch */
    global_cond = 150;  /* > threshold */
    sum += target_function(10, 20);
    
    /* Test 2: Take the else-branch */
    global_cond = 50;   /* < threshold */
    sum += target_function(30, 40);
    
    /* Test 3: Exercise second function */
    sum += target_function2();
    sum += target_function2();  /* Different path on second call */
    
    /* Test 4: Exercise third function with various inputs */
    for (int i = 0; i < 10; i++) {
        sum += target_function3(i, i*2, i*3);
    }
    
    /* Vary global_cond to trigger different optimization decisions */
    for (int i = 0; i < 100; i += 7) {
        global_cond = i;
        *global_ptr = (i % 3) ? 1 : 0;  /* Alternate zero/non-zero */
        sum += target_function(i, i+1);
    }
    
    printf("Result: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
