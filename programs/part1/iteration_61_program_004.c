#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Function to accumulate results */
int accumulate_result(int base, int value) {
    return base + value;
}

/* Main function containing various do-while patterns */
int main() {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent optimization */
    
    /* PATTERN 1: Basic signed int decrement pattern (should match) */
    printf("Pattern 1: Basic signed int decrement\n");
    {
        int counter = 100;
        do {
            total += 1;
            bar();  /* External call prevents optimization */
        } while (--counter > 0);
    }
    
    /* PATTERN 2: Unsigned int decrement pattern (should match) */
    printf("Pattern 2: Unsigned int decrement\n");
    {
        unsigned int u_counter = 50;
        do {
            total += 2;
            vol_total += 1;
        } while (--u_counter != 0);
    }
    
    /* PATTERN 3: register qualified variable (should match) */
    printf("Pattern 3: Register qualified counter\n");
    {
        register int reg_counter = 25;
        do {
            total = accumulate_result(total, 3);
        } while ((reg_counter -= 1) != 0);
    }
    
    /* PATTERN 4: short type counter (should match) */
    printf("Pattern 4: Short type counter\n");
    {
        short s_counter = 10;
        do {
            total += 4;
            bar();
        } while (--s_counter > 0);
    }
    
    /* PATTERN 5: char type counter (should match) */
    printf("Pattern 5: Char type counter\n");
    {
        char c_counter = 5;
        do {
            total += 5;
        } while (--c_counter != 0);
    }
    
    /* PATTERN 6: Counter starts at 1 (edge case) */
    printf("Pattern 6: Counter starts at 1\n");
    {
        int edge_counter = 1;
        do {
            total += 6;
        } while (--edge_counter > 0);
    }
    
    /* PATTERN 7: Loop inside if statement */
    printf("Pattern 7: Loop inside if\n");
    {
        int if_counter = 15;
        if (total > 0) {
            do {
                total += 7;
                bar();
            } while (--if_counter > 0);
        }
    }
    
    /* PATTERN 8: Function parameter as counter */
    printf("Pattern 8: Function parameter counter\n");
    {
        int param_counter = 20;
        void nested_func(int cnt) {
            do {
                total += 8;
            } while (--cnt > 0);
        }
        nested_func(param_counter);
    }
    
    /* PATTERN 9: Post-increment (should NOT match - tests GEN_INT(-1) check) */
    printf("Pattern 9: Post-increment (should not match pattern)\n");
    {
        int post_counter = 10;
        do {
            total += 9;
        } while (post_counter++ < 20);
    }
    
    /* PATTERN 10: Compare against non-zero (should NOT match - tests const0_rtx check) */
    printf("Pattern 10: Compare against non-zero\n");
    {
        int non_zero_counter = 10;
        do {
            total += 10;
        } while (--non_zero_counter > 5);
    }
    
    /* PATTERN 11: volatile counter (may inhibit pattern) */
    printf("Pattern 11: Volatile counter\n");
    {
        volatile int vol_counter = 8;
        do {
            total += 11;
        } while (--vol_counter > 0);
    }
    
    /* PATTERN 12: Complex body with pointer */
    printf("Pattern 12: Complex body with pointer\n");
    {
        int ptr_counter = 12;
        int data[12] = {0};
        int *ptr = data;
        do {
            *ptr++ = total;
            total += 12;
        } while (--ptr_counter > 0);
    }
    
    /* PATTERN 13: Nested decrement pattern */
    printf("Pattern 13: Multiple loops in sequence\n");
    {
        int outer = 3;
        do {
            int inner = 4;
            do {
                total += 13;
                bar();
            } while (--inner > 0);
        } while (--outer > 0);
    }
    
    /* Final result */
    printf("Final total: %d\n", total);
    printf("Volatile total: %d\n", vol_total);
    
    return total > 0 ? 0 : 1;
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
