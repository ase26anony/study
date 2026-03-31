/* ifcvt_test.c - Target the uncovered lines in GCC's if-conversion pass */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex condition expressions */
static volatile int global_cond = 0;
static volatile int global_threshold = 100;
static volatile int* volatile global_ptr = NULL;
static int static_counter = 0;

/* Prevent optimization of the test function */
__attribute__((noinline, noclone))
int test_if_conversion(int x, int y) {
    volatile int local_mod = x;
    int result = 0;
    
    /* Complex condition using global variable and pointer dereference */
    /* This should generate a non-trivial RTL test_expr */
    if (global_cond < global_threshold && 
        (global_ptr != NULL ? *global_ptr : 0) > 0) {
        
        /* This is the critical part: modifying the condition variable
           BEFORE any real instruction in the then block */
        
        /* First, generate some NOTE/DEBUG_INSN instructions in the header */
        /* These appear as NOTE or DEBUG_INSN in RTL */
        asm volatile("# HEADER NOTE 1" : : : "memory");
        asm volatile("# HEADER NOTE 2" : : : "memory");
        
        /* Now modify the global variable used in the condition */
        /* This instruction should be in the header before then_last_head */
        global_cond = x + y;  /* MODIFIES CONDITION EXPRESSION */
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* More notes/debug insns after modification */
        asm volatile("# HEADER NOTE 3" : : : "memory");
        
        /* Now the "real" instructions begin - this marks then_last_head */
        result = x * y + global_cond;
        
        /* Additional instructions to make block non-trivial */
        for (int i = 0; i < 3; i++) {
            result += i * global_cond;
        }
        
        /* Function call to prevent tail merging */
        static_counter += result % 10;
        
        /* Store to volatile to prevent dead code elimination */
        local_mod = result;
        
    } else {
        /* Else branch with different computation */
        result = x - y;
        global_cond = result / 2;
    }
    
    return result + static_counter;
}

/* Another test case with different condition pattern */
__attribute__((noinline, noclone))
int test_pointer_modification(int* ptr1, int* ptr2) {
    int temp = 0;
    
    /* Condition involving pointer dereference */
    if (ptr1 != NULL && *ptr1 > *ptr2) {
        
        /* Notes in header */
        asm volatile("# PTR HEADER NOTE" : : : "memory");
        
        /* Modify through pointer - affects condition expression */
        *ptr1 = *ptr2 - 10;  /* MODIFIES DEREFERENCED VALUE */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* More notes */
        asm volatile("# ANOTHER NOTE" : : : "memory");
        
        /* Real computation starts here */
        temp = *ptr1 * *ptr2;
        
        /* Additional operations */
        for (int i = 0; i < 4; i++) {
            temp += i;
            asm volatile("# LOOP BODY" : : : "memory");
        }
        
    } else {
        temp = (*ptr2) * 2;
        if (ptr1) *ptr1 = temp;
    }
    
    return temp;
}

/* Test with static variable condition */
static volatile int static_cond = 50;
__attribute__((noinline, noclone))
int test_static_condition(int a, int b) {
    int sum = 0;
    
    /* Static variable in condition */
    if (static_cond > 25 && a > b) {
        
        /* Multiple notes to ensure header section */
        asm volatile("# STATIC NOTE 1" : : : "memory");
        asm volatile("# STATIC NOTE 2" : : : "memory");
        asm volatile("# STATIC NOTE 3" : : : "memory");
        
        /* Modify static condition variable */
        static_cond = a - b;  /* MODIFIES CONDITION */
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Another note */
        asm volatile("# AFTER MODIFICATION" : : : "memory");
        
        /* First real instruction */
        sum = a + b + static_cond;
        
        /* More computation */
        sum *= 2;
        sum -= static_cond;
        
        /* Volatile operation */
        asm volatile("" : "+r" (sum) : : "memory");
        
    } else {
        sum = b - a;
        static_cond = sum;
    }
    
    return sum;
}

int main(void) {
    int array1[2] = {200, 100};
    int array2[2] = {50, 150};
    int* ptr1 = array1;
    int* ptr2 = array2;
    
    int total = 0;
    
    /* Initialize globals */
    global_cond = 50;
    global_threshold = 100;
    global_ptr = &array1[0];
    
    /* Call test functions multiple times with different inputs
       to exercise different paths and trigger if-conversion */
    for (int i = 0; i < 10; i++) {
        /* Vary inputs to take both branches */
        if (i % 3 == 0) {
            global_cond = 150;  /* Will take else branch */
        } else {
            global_cond = 50;   /* Will take then branch */
        }
        
        /* Modify pointer target */
        *global_ptr = (i % 2 == 0) ? 200 : 0;
        
        /* Call first test */
        total += test_if_conversion(i * 10, i * 5);
        
        /* Call second test with varying pointer values */
        ptr1 = (i % 4 == 0) ? NULL : array1;
        total += test_pointer_modification(ptr1, ptr2);
        
        /* Call third test */
        total += test_static_condition(i, i * 2);
        
        /* Modify arrays */
        array1[0] += i;
        array2[0] -= i;
    }
    
    printf("Total result: %d\n", total);
    printf("Static counter: %d\n", static_counter);
    printf("Global cond: %d\n", global_cond);
    printf("Static cond: %d\n", static_cond);
    
    return total != 0 ? 0 : 1;
}
