/* ifcvt-test.c
 * Target: Trigger if-conversion safety check for header modifications
 * Compile with: gcc -O2 -fdump-rtl-ifcvt -fdump-rtl-all ifcvt-test.c -o ifcvt-test
 * For ARM: gcc -O3 -march=armv7-a -mtune=cortex-a9 ifcvt-test.c -o ifcvt-test-arm
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create non-trivial condition expressions */
static volatile int global_cond = 0;
static int* volatile global_ptr = NULL;
static int global_array[4] = {0, 0, 0, 0};

/* Function to prevent optimization */
static int __attribute__((noinline, noclone)) 
side_effect(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x + 1;
}

/* Function with complex condition and early modification in then-block */
static int __attribute__((noinline, noclone, optimize("O2")))
test_if_conversion(int a, int b, int threshold) {
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    if (global_cond > threshold && *global_ptr != 0) {
        /* This assignment modifies global_cond which is part of the condition.
         * It should appear in the header before then_last_head */
        global_cond = a + b;  /* MODIFICATION OF CONDITION VARIABLE */
        
        /* Debug/Note instructions - will generate NOTE or DEBUG_INSN in RTL */
        asm volatile("# DEBUG/NOTE: Entering then block");
        asm volatile("# Another note instruction");
        
        /* Additional statements to create a multi-instruction block */
        result = side_effect(a);
        global_array[0] = result * 2;
        global_array[1] = side_effect(b);
        
        /* More operations to ensure block isn't optimized away */
        asm volatile("" : : : "memory");
        result += global_array[2];
    } else {
        /* Else block with different operations */
        result = side_effect(b - a);
        global_array[3] = result;
        asm volatile("# Else block note");
    }
    
    /* Use result to prevent dead code elimination */
    return result + global_cond;
}

/* Another test case with different condition pattern */
static int __attribute__((noinline, noclone))
test_pointer_modification(int* ptr1, int* ptr2) {
    int local_result = 0;
    
    /* Condition involving pointer comparison and dereference */
    if (ptr1 != ptr2 && *ptr1 > *ptr2) {
        /* Modify *ptr1 which is used in the condition */
        *ptr1 = *ptr2 - 1;  /* MODIFICATION OF DEREFERENCED POINTER */
        
        /* Notes/debug instructions */
        asm volatile("# Note: Pointer modification block");
        asm volatile("# Additional note");
        
        /* Body of then block */
        local_result = *ptr1 + *ptr2;
        global_cond = side_effect(local_result);
        
        /* More operations */
        asm volatile("" : : "r"(ptr1), "r"(ptr2) : "memory");
        local_result += global_array[0];
    } else {
        local_result = *ptr2 - *ptr1;
        asm volatile("# Note: Else path");
    }
    
    return local_result;
}

/* Main function to exercise different paths */
int main(void) {
    int data1 = 10, data2 = 20;
    int results[4];
    int i;
    
    /* Initialize globals */
    global_cond = 5;
    global_ptr = &data1;
    data1 = 15;  /* Make *global_ptr != 0 */
    
    /* Test 1: Should take then-branch */
    printf("Test 1 - then branch:\n");
    results[0] = test_if_conversion(3, 4, 2);
    printf("  Result: %d, global_cond: %d\n", results[0], global_cond);
    
    /* Test 2: Should take else-branch (global_cond <= threshold) */
    global_cond = 1;
    printf("\nTest 2 - else branch (global_cond <= threshold):\n");
    results[1] = test_if_conversion(3, 4, 2);
    printf("  Result: %d, global_cond: %d\n", results[1], global_cond);
    
    /* Test 3: Should take else-branch (*global_ptr == 0) */
    global_cond = 5;
    data1 = 0;
    printf("\nTest 3 - else branch (*ptr == 0):\n");
    results[2] = test_if_conversion(3, 4, 2);
    printf("  Result: %d\n", results[2]);
    
    /* Test 4: Pointer modification test */
    printf("\nTest 4 - pointer modification:\n");
    int x = 100, y = 50;
    results[3] = test_pointer_modification(&x, &y);
    printf("  Result: %d, x: %d, y: %d\n", results[3], x, y);
    
    /* Additional loop to increase execution frequency */
    for (i = 0; i < 100; i++) {
        global_cond = i % 10;
        data1 = (i % 3) ? 1 : 0;
        test_if_conversion(i, i*2, 5);
    }
    
    return 0;
}
