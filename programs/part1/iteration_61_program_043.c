#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to accumulate results */
int global_sum = 0;

/* Test various do-while patterns */
void test_loops(int param_counter) {
    int i;
    unsigned int u;
    short s;
    char c;
    int *ptr;
    int local_sum = 0;
    
    /* Pattern 1: Basic signed int decrement */
    i = 10;
    do {
        local_sum += i;
        bar();
    } while (--i > 0);
    
    /* Pattern 2: Unsigned int decrement with != 0 */
    u = 10;
    do {
        local_sum += u;
        *(&local_sum) = local_sum;  /* Simple side effect */
    } while (--u != 0);
    
    /* Pattern 3: Short type with register qualifier */
    register short rs = 8;
    do {
        local_sum += rs;
        bar();
    } while (--rs > 0);
    
    /* Pattern 4: Char type */
    c = 6;
    do {
        local_sum += c;
    } while (--c > 0);
    
    /* Pattern 5: Counter starts at 1 (executes once) */
    i = 1;
    do {
        local_sum += 100;
        bar();
    } while (--i > 0);
    
    /* Pattern 6: Function parameter as counter */
    if (param_counter > 0) {
        do {
            local_sum += param_counter;
            bar();
        } while (--param_counter > 0);
    }
    
    /* Pattern 7: Counter in if statement branch */
    if (local_sum > 0) {
        i = 5;
        do {
            local_sum *= 2;
        } while (--i > 0);
    }
    
    /* Pattern 8: Followed by other statements */
    i = 7;
    do {
        local_sum += i * 2;
    } while (--i > 0);
    
    /* Additional statement affecting register allocation */
    int temp = local_sum * 3;
    local_sum = temp / 2;
    
    /* Pattern 9: Using volatile (should NOT match pattern) */
    volatile int vi = 4;
    do {
        local_sum += vi;
    } while (--vi > 0);
    
    /* Pattern 10: Compound assignment */
    i = 9;
    do {
        local_sum += i;
        bar();
    } while ((i -= 1) != 0);
    
    /* Pattern that should NOT match: post-increment */
    i = 3;
    do {
        local_sum += i;
    } while (i++ < 5);  /* Should fail cmp_arg2 != const0_rtx check */
    
    /* Pattern that should NOT match: compare against non-zero */
    i = 5;
    do {
        local_sum += i;
    } while (--i > 2);  /* Should fail cmp_arg2 != const0_rtx check */
    
    global_sum += local_sum;
}

/* Additional test functions with different contexts */
void test_nested_context(void) {
    int outer = 3;
    while (outer-- > 0) {
        int inner = 4;
        /* Target do-while inside while loop */
        do {
            global_sum += inner;
            bar();
        } while (--inner > 0);
    }
}

void test_with_pointer(void) {
    int counter = 5;
    int data[10];
    int *ptr = data;
    
    do {
        *ptr++ = counter;
        bar();
    } while (--counter > 0);
    
    /* Verify we wrote something */
    for (int i = 0; i < 5; i++) {
        global_sum += data[i];
    }
}

int main(void) {
    /* Initialize with predictable values */
    global_sum = 0;
    
    /* Test various loop patterns */
    test_loops(5);
    test_nested_context();
    test_with_pointer();
    
    /* Additional direct tests in main */
    {
        /* Pattern: register variable with different type */
        register unsigned char rc = 10;
        int local_acc = 0;
        
        do {
            local_acc += rc;
            bar();
        } while (--rc != 0);
        
        global_sum += local_acc;
    }
    
    {
        /* Pattern: counter as local with complex body */
        int i = 7;
        int arr[7];
        
        do {
            arr[i] = i * i;
            global_sum += arr[i];
            bar();
        } while (--i > 0);
    }
    
    /* Print verifiable result */
    printf("Result: %d\n", global_sum);
    
    return 0;
}

/* Dummy implementation of bar() to avoid linker errors */
void bar(void) {
    /* Minimal side effect */
    static int call_count = 0;
    call_count++;
}
