#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Global pointer for memory operations */
volatile int *global_ptr;

/* Function with parameter counter */
int loop_with_param(int counter) {
    int sum = 0;
    do {
        sum += counter;
        bar();
    } while (--counter > 0);
    return sum;
}

int main(void) {
    int total = 0;
    volatile int *ptr = &total;
    
    /* Test 1: Basic signed int counter with simple body */
    {
        int counter = 100;
        int sum = 0;
        do {
            sum += 1;
            *ptr = 0;  /* Simple side effect */
        } while (--counter > 0);
        total += sum;
    }
    
    /* Test 2: Unsigned int counter with != 0 comparison */
    {
        unsigned int u_counter = 50;
        int sum = 0;
        do {
            sum += 2;
            bar();  /* External function call */
        } while (--u_counter != 0);
        total += sum;
    }
    
    /* Test 3: register-qualified char counter */
    {
        register char counter = 25;
        int sum = 0;
        do {
            sum += 3;
            asm volatile("" : : : "memory");  /* Memory barrier */
        } while ((counter -= 1) != 0);
        total += sum;
    }
    
    /* Test 4: short counter inside if statement */
    {
        short counter = 10;
        int sum = 0;
        if (total > -100) {
            do {
                sum += 4;
                bar();
            } while (--counter > 0);
        }
        total += sum;
    }
    
    /* Test 5: Edge case - counter starts at 1 (executes once) */
    {
        int counter = 1;
        int sum = 0;
        do {
            sum += 5;
            *ptr = 1;
        } while (--counter > 0);
        total += sum;
    }
    
    /* Test 6: Function parameter as counter */
    total += loop_with_param(20);
    
    /* Test 7: Different storage - automatic with complex expression */
    {
        int counter = 15;
        int sum = 0;
        int *local_ptr = &sum;
        do {
            *local_ptr += 6;
            bar();
        } while (--counter > 0);
        total += sum;
    }
    
    /* Test 8: Counter with post-increment (SHOULD NOT match pattern) */
    {
        int counter = 5;
        int sum = 0;
        do {
            sum += 7;
            counter++;  /* Post-increment, not decrement */
        } while (counter < 10);  /* Different comparison */
        total += sum;
    }
    
    /* Test 9: Counter comparing against non-zero (SHOULD NOT match) */
    {
        int counter = 8;
        int sum = 0;
        do {
            sum += 8;
        } while (--counter > 5);  /* Compare against 5, not 0 */
        total += sum;
    }
    
    /* Test 10: volatile counter (likely won't match but tests edge) */
    {
        volatile int counter = 3;
        int sum = 0;
        do {
            sum += 9;
            bar();
        } while (--counter > 0);
        total += sum;
    }
    
    /* Test 11: Nested context with multiple statements after */
    {
        int counter = 12;
        int sum = 0;
        do {
            sum += 10;
            global_ptr = &sum;
        } while (--counter > 0);
        
        /* Additional statements affecting register allocation */
        int temp = sum * 2;
        total += temp;
    }
    
    /* Test 12: Different integer type - long counter */
    {
        long counter = 7;
        int sum = 0;
        do {
            sum += 11;
            bar();
        } while (--counter > 0);
        total += sum;
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    static int call_count = 0;
    call_count++;
}
