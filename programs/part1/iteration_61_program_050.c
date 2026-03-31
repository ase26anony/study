#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Global pointer for side effects */
int global_counter = 0;

/* Function with parameter counter */
void loop_with_param(int param_counter) {
    int local_sum = 0;
    
    /* Pattern 1: Basic signed int decrement */
    int counter1 = 100;
    do {
        local_sum += counter1;
        bar();
    } while (--counter1 > 0);
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    unsigned int counter2 = 50;
    do {
        local_sum += counter2;
        global_counter++;
    } while (--counter2 != 0);
    
    /* Pattern 3: register qualified variable */
    register int counter3 = 75;
    do {
        local_sum += counter3;
        bar();
    } while (--counter3 > 0);
    
    /* Pattern 4: short type */
    short counter4 = 25;
    do {
        local_sum += counter4;
        global_counter--;
    } while (--counter4 > 0);
    
    /* Pattern 5: char type */
    char counter5 = 10;
    do {
        local_sum += counter5;
        bar();
    } while (--counter5 > 0);
    
    /* Pattern 6: Using function parameter */
    do {
        local_sum += param_counter;
        global_counter += 2;
    } while (--param_counter > 0);
    
    printf("Param loop sum: %d\n", local_sum);
}

/* Main function with various loop patterns */
int main() {
    int total = 0;
    int *ptr = &total;
    
    /* Pattern 7: Basic loop in main with pointer side effect */
    int counter7 = 200;
    do {
        *ptr += counter7;
        bar();
    } while (--counter7 > 0);
    
    /* Pattern 8: Loop inside if statement */
    if (global_counter < 1000) {
        int counter8 = 150;
        do {
            total += counter8;
            global_counter++;
        } while (--counter8 > 0);
    }
    
    /* Pattern 9: Unsigned with compound assignment */
    unsigned int counter9 = 80;
    do {
        total += counter9;
        bar();
    } while ((counter9 -= 1) != 0);
    
    /* Pattern 10: Counter starting at 1 (executes once) */
    int counter10 = 1;
    do {
        total += 1000;
        bar();
    } while (--counter10 > 0);
    
    /* Pattern 11: Loop followed by other statements */
    int counter11 = 60;
    do {
        total += counter11;
        global_counter--;
    } while (--counter11 > 0);
    
    /* Additional statements affecting register allocation */
    int temp = total * 2;
    total = temp / 2;
    
    /* Pattern 12: volatile counter (should NOT match pattern) */
    volatile int counter12 = 30;
    do {
        total += 5;
        bar();
    } while (--counter12 > 0);
    
    /* Call function with parameter loop */
    loop_with_param(40);
    
    /* NON-MATCHING PATTERNS (for boundary testing) */
    
    /* Pattern A: Post-increment (should NOT match GEN_INT(-1) check) */
    int counterA = 20;
    do {
        total += 1;
        bar();
    } while (counterA++ < 30);
    
    /* Pattern B: Compare against non-zero (should fail const0_rtx check) */
    int counterB = 15;
    do {
        total += 2;
        bar();
    } while (--counterB > 5);
    
    /* Pattern C: Different comparison operator */
    int counterC = 25;
    do {
        total += 3;
        global_counter++;
    } while (--counterC >= 1);
    
    printf("Final total: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total > 0 ? 0 : 1;
}

/* Dummy implementation of bar() to avoid linker errors */
void bar(void) {
    /* Minimal side effect */
    static int call_count = 0;
    call_count++;
}
