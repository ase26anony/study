#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to accumulate results */
int main() {
    int total = 0;
    volatile int* ptr = &total;  /* Use volatile pointer for side effects */
    
    /* Pattern 1: Basic signed int counter with > 0 condition */
    {
        int counter = 100;
        do {
            total += 1;
            bar();  /* External call prevents dead code elimination */
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int counter with != 0 condition */
    {
        unsigned int u_counter = 50;
        do {
            *ptr = *ptr + 2;  /* Use volatile pointer access */
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: Short type counter in register storage class */
    {
        register short reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: Char type counter with explicit decrement */
    {
        char char_counter = 10;
        do {
            total += 4;
        } while ((char_counter -= 1) != 0);
    }
    
    /* Pattern 5: Counter as function parameter (simulated) */
    {
        int param_counter = 5;
        /* Simulate parameter by using it in a separate scope */
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* Pattern 6: Counter starting at 1 (executes once) */
    {
        int once_counter = 1;
        do {
            total += 6;
        } while (--once_counter > 0);
    }
    
    /* Pattern 7: Loop inside if statement */
    {
        int if_counter = 8;
        if (total < 1000) {
            do {
                total += 7;
                bar();
            } while (--if_counter > 0);
        }
    }
    
    /* Pattern 8: Counter with post-increment (SHOULD NOT match pattern) */
    {
        int post_counter = 3;
        do {
            total += 8;
        } while (post_counter++ < 3);  /* Different pattern - should fail cmp_arg2 != const0_rtx */
    }
    
    /* Pattern 9: Counter comparing against non-zero (SHOULD NOT match pattern) */
    {
        int non_zero_counter = 4;
        do {
            total += 9;
        } while (--non_zero_counter > 2);  /* Should fail cmp_arg2 != const0_rtx check */
    }
    
    /* Pattern 10: Volatile counter (likely won't match but tests edge case) */
    {
        volatile int volatile_counter = 6;
        do {
            total += 10;
        } while (--volatile_counter > 0);
    }
    
    /* Pattern 11: Complex body but simple decrement */
    {
        int complex_counter = 7;
        int local_sum = 0;
        do {
            total += 11;
            local_sum += complex_counter;  /* Additional computation */
            bar();
        } while (--complex_counter > 0);
    }
    
    /* Pattern 12: Followed by other statements affecting register allocation */
    {
        int alloc_counter = 9;
        int temp1 = 0, temp2 = 0, temp3 = 0;
        do {
            total += 12;
            temp1 = alloc_counter * 2;
            temp2 = temp1 + 1;
            temp3 = temp2 - alloc_counter;
        } while (--alloc_counter > 0);
        /* Additional statements that might affect liveness */
        temp1 = temp2 + temp3;
    }
    
    printf("Result: %d\n", total);
    return total;
}

/* Dummy implementation of bar() if linking standalone */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
