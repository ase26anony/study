/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create complex enough control flow */
int fill_delay_slot_test(int argc, char **argv) {
    volatile int trigger = argc; /* Prevent constant propagation */
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Loop to create scheduling context */
    for (int i = 0; i < (argc > 1 ? 100 : 200); ++i) {
        /* Mix of operations to create register pressure */
        int temp = b * c;
        d = temp - a;
        
        /* KEY CONSTRUCT: Conditional jump with potential delay slot candidate */
        if ((i % 7) == (trigger & 0x3)) { /* Runtime-dependent condition */
            /* Jump to label where safe instruction resides */
            goto target_label;
        }
        
        /* Some other operations to prevent optimization */
        a = b + d;
        b = c ^ i;
        continue;
        
    target_label:
        /* SAFE CANDIDATE INSTRUCTION for delay slot filling */
        /* Simple arithmetic, no trapping, no resource conflicts */
        a = b + c;  /* This should compile to simple add instruction */
        
        /* Additional operations so target isn't isolated */
        c = d * 2;
        d = a ^ b;
    }
    
    /* Use results to prevent dead code elimination */
    result = a + b + c + d;
    printf("Result: %d\n", result);
    
    return result;
}

/* Helper function to create more complex control flow */
void additional_operations(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i * i;
    }
}

int main(int argc, char **argv) {
    int arr[50];
    
    /* Create some side effects */
    additional_operations(arr, 50);
    
    /* Main test */
    int res = fill_delay_slot_test(argc, argv);
    
    /* Use array to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        sum += arr[i];
    }
    
    return (res + sum) & 0xFF;
}
