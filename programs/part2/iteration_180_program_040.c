/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: mips64-linux-gnu-gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent control flow */
int fill_delay_slot_test(int argc, char **argv) {
    volatile int trigger = argc; /* Prevent constant propagation */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Loop to create scheduling context */
    int limit = (argc > 1) ? 100 : 200;
    for (int i = 0; i < limit; ++i) {
        /* Create a conditional jump that's not always taken */
        if ((i + trigger) % 7 == 0) {
            /* Simple conditional jump to label */
            goto target_label;
        }
        
        /* Some intermediate computation to separate blocks */
        d = a + b;
        continue;
        
    target_label:
        /* Candidate instruction for delay slot filling:
           Simple, non-trapping, resource-independent operation */
        a = b + c;  /* Should compile to simple add instruction */
        
        /* Additional instruction so target isn't isolated */
        b = c * d;
        
        /* Prevent loop optimization */
        if (i % 13 == 0) {
            c = d - a;
        }
    }
    
    /* Use all variables to prevent dead code elimination */
    result = a + b + c + d;
    
    /* Create observable side effect */
    printf("Result: %d (trigger: %d)\n", result, trigger);
    
    return result;
}

/* Additional function to increase complexity */
void setup_variables(int *arr, int n) {
    for (int i = 0; i < n; ++i) {
        arr[i] = i * 2;
    }
}

int main(int argc, char **argv) {
    int arr[10];
    
    /* Initialize with runtime-dependent values */
    setup_variables(arr, 10);
    
    /* Call the test function multiple times with different conditions */
    int total = 0;
    for (int j = 0; j < 3; ++j) {
        total += fill_delay_slot_test(argc + j, argv);
    }
    
    /* Additional control flow to prevent optimization */
    if (total > 1000) {
        printf("Unexpected large total: %d\n", total);
    }
    
    return total % 256;
}
