#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to use loop counter as parameter */
void loop_with_param(int counter) {
    int sum = 0;
    do {
        sum += counter;
        bar();
    } while (--counter > 0);
    printf("Param loop sum: %d\n", sum);
}

int main() {
    int total = 0;
    
    /* Pattern 1: Basic int counter with > 0 condition */
    {
        int counter = 10;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int counter with != 0 condition */
    {
        unsigned int u_counter = 10;
        do {
            total += 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: Short counter with register qualifier */
    {
        register short s_counter = 10;
        do {
            total += 3;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Pattern 4: Char counter with simple body */
    {
        char c_counter = 10;
        char local_sum = 0;
        do {
            local_sum += c_counter;
            bar();
        } while (--c_counter > 0);
        total += local_sum;
    }
    
    /* Pattern 5: Counter starts at 1 (executes once) */
    {
        int counter = 1;
        do {
            total += 100;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 6: Counter as local pointer manipulation */
    {
        int counter = 5;
        int *ptr = &counter;
        do {
            *ptr = 0;  /* Simple side effect */
            total += 10;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 7: Inside if statement */
    {
        int flag = 1;
        if (flag) {
            int counter = 7;
            do {
                total += 7;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* Pattern 8: Followed by other statements */
    {
        int counter = 6;
        int temp = 0;
        do {
            temp += counter;
            bar();
        } while (--counter > 0);
        total += temp;
    }
    
    /* Pattern 9: Using -= operator instead of -- */
    {
        int counter = 8;
        do {
            total += 8;
            bar();
        } while ((counter -= 1) != 0);
    }
    
    /* Pattern 10: Mixed with other control flow */
    {
        int counter = 4;
        int i;
        for (i = 0; i < 2; i++) {
            do {
                total += 4;
                bar();
            } while (--counter > 0);
            counter = 4;  /* Reset for second iteration */
        }
    }
    
    /* NON-MATCHING PATTERNS (should fail the checks) */
    
    /* Pattern A: Post-increment (should not match GEN_INT(-1)) */
    {
        int counter = 3;
        do {
            total += 20;
            bar();
        } while (counter++ < 5);
    }
    
    /* Pattern B: Compare against non-zero (should fail const0_rtx check) */
    {
        int counter = 10;
        do {
            total += 30;
            bar();
        } while (--counter > 5);
    }
    
    /* Pattern C: Volatile counter (may inhibit pattern) */
    {
        volatile int v_counter = 3;
        do {
            total += 40;
            bar();
        } while (--v_counter > 0);
    }
    
    /* Pattern D: Complex expression in condition */
    {
        int counter = 4;
        int limit = 0;
        do {
            total += 50;
            bar();
        } while (--counter > limit);
    }
    
    /* Call function with parameter loop */
    loop_with_param(5);
    
    printf("Final total: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
