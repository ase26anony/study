/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile control variables */
volatile int cond1 = 0;
volatile int cond2 = 1;
volatile int cond3 = 2;
volatile int result = 0;

/* Function with attribute to ensure O2 optimization */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 30;
    volatile int x = 0, y = 0, z = 0;
    
    /* Create resource set for parent instruction */
    x = a + b;  /* Parent instruction for delay slot */
    
    /* External call to create resource barrier */
    ext_func1(x);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    y = b + c;
    ext_func2(y);
    
    /* Avoid falling through to label */
    if (cond2) {
        z = x + y;
        goto end1;
    }
    
target_label1:
    /* Non-jump, non-sequence instruction at jump target */
    /* Uses different variables than parent instruction */
    c = a + 5;  /* next_trial candidate */
    
end1:
    /* Use result to prevent optimization */
    result += x + y + z + c;
    
    /* Memory clobber to prevent reordering */
    __asm__ volatile ("" : : : "memory");
}

/* Another pattern with switch statement */
__attribute__((optimize("O2"), noinline))
void test_pattern2(void) {
    volatile int i = 0, j = 0, k = 0;
    volatile int temp = 0;
    
    /* Parent instruction with its own resource set */
    i = ext_func1(cond3);
    
    /* Switch to create complex control flow */
    switch (cond2) {
        case 1:
            if (cond1) {
                goto target_label2;
            }
            j = i * 2;
            break;
        case 2:
            k = i + 100;
            break;
        default:
            temp = 999;
    }
    
    /* More code to separate label */
    ext_func2(j);
    
    if (k > 50) {
        temp = k - 20;
        goto end2;
    }
    
target_label2:
    /* Safe arithmetic operation at target */
    /* No overlap with parent's resources (i vs j) */
    j = 42;  /* next_trial candidate */
    
end2:
    result += i + j + k + temp;
    __asm__ volatile ("" : : : "memory");
}

/* Pattern with loop and computed goto */
__attribute__((optimize("O2")))
void test_pattern3(void) {
    volatile int counter = 0;
    volatile int sum = 0;
    volatile int data[4] = {1, 2, 3, 4};
    
    /* Parent instruction */
    sum = data[0] + data[1];
    
    /* External call */
    ext_func3(sum);
    
    /* Loop with conditional jump to label */
    while (counter < 3) {
        volatile int local = data[counter];
        
        if (cond1 && (counter == 1)) {
            static void* labels[] = { &&loop_end, &&target_label3 };
            goto *labels[cond2 % 2];
        }
        
        sum += local;
        counter++;
        continue;
        
target_label3:
        /* Safe operation at target - different resource than parent */
        data[3] = local + 1;  /* next_trial candidate */
        break;
    }
    
loop_end:
    result += sum + data[3];
    __asm__ volatile ("" : : : "memory");
}

/* Pattern with nested conditionals */
__attribute__((optimize("O3"), noinline))
void test_pattern4(void) {
    volatile int p = 0, q = 0, r = 0;
    volatile int accum = 0;
    
    /* Parent instruction using specific registers/variables */
    p = cond1 * 7;
    
    /* Multiple external calls to create barriers */
    ext_func1(p);
    ext_func2(p + 1);
    
    /* Complex conditional structure */
    if (cond1 > 0) {
        if (cond2 < 10) {
            if (cond3 != 5) {
                goto target_label4;
            }
            q = p / 2;
        } else {
            r = p * 3;
        }
    } else {
        accum = 100;
    }
    
    /* More separating code */
    for (int i = 0; i < 2; i++) {
        accum += i;
        ext_func3(accum);
    }
    
    if (accum > 50) {
        goto end4;
    }
    
target_label4:
    /* Simple assignment with no trapping */
    /* Uses variable not touched by parent */
    r = 25;  /* next_trial candidate */
    
end4:
    result += p + q + r + accum;
    __asm__ volatile ("" : : : "memory");
}

/* Pattern designed for MIPS delay slots specifically */
__attribute__((optimize("O2"), noinline))
void test_mips_pattern(void) {
    volatile int reg_a = 0, reg_b = 0, reg_c = 0;
    volatile int reg_t = 0, reg_s = 0;
    
    /* Parent instruction - could be in delay slot */
    reg_a = reg_b + reg_c;
    
    /* Force conditional branch */
    if (cond1) {
        /* Inline asm to prevent optimization */
        __asm__ volatile (
            "nop \n\t"
            "nop \n\t"
            : : : "memory"
        );
        goto mips_target;
    }
    
    /* Different computation path */
    reg_t = ext_func1(reg_a);
    
    if (reg_t > 0) {
        reg_s = reg_t * 2;
        goto mips_end;
    }
    
mips_target:
    /* Candidate for delay slot filling */
    /* No resource conflict with parent */
    reg_c = reg_b + 10;  /* next_trial candidate */
    
mips_end:
    result += reg_a + reg_b + reg_c + reg_t + reg_s;
}

/* Main function to execute all patterns */
int main(void) {
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 2;
    cond3 = 3;
    
    /* Execute all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_mips_pattern();
    
    /* Print result to ensure execution */
    printf("Result: %d\n", result);
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x * 2; }
int ext_func3(int x) { return x - 1; }
