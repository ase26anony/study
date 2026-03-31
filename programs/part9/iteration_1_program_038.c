#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    const int *end = arr + n;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    while (p < end) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* increment in separate statement */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Tight loop with dereference and increment */
    while (n-- > 0) {
        *d = *s;        /* Simple (mem (reg)) access */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 3: Short pointer with explicit split operations */
short process_shorts(const short *data, int count) {
    const short *ptr = data;
    short result = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        short val = *ptr;   /* Load via register */
        ptr = ptr + 1;      /* Increment separately */
        result ^= val;      /* Some computation */
    }
    return result;
}

/* Pattern 4: Int pointer with while loop and post-increment */
void fill_sequence(int *buffer, int size, int start) {
    int *p = buffer;
    int value = start;
    
    while (size-- > 0) {
        *p = value;         /* Store via register */
        p = p + 1;          /* Increment separately */
        value += 2;
    }
}

/* Pattern 5: Mixed operations in same basic block */
int sum_and_copy(int *restrict dst, const int *restrict src, int n) {
    const int *s = src;
    int *d = dst;
    int total = 0;
    
    /* Multiple (mem (reg)) patterns in one loop */
    while (n-- > 0) {
        int val = *s;       /* Load via register */
        s = s + 1;          /* Increment source */
        total += val;
        *d = val;           /* Store via register */
        d = d + 1;          /* Increment destination */
    }
    return total;
}

/* Pattern 6: Simple pointer traversal in main */
int traverse_with_pointer(const int *arr, int n) {
    const int *ptr = arr;
    int sum = 0;
    
    /* Very basic pattern: *ptr followed by ptr++ */
    for (int i = 0; i < n; i++) {
        sum += *ptr;    /* Should be (mem (reg ptr)) */
        ptr = ptr + 1;  /* Separate increment */
    }
    return sum;
}

/* Pattern 7: Byte-wise processing with char pointer */
unsigned char checksum(const unsigned char *data, int length) {
    const unsigned char *p = data;
    unsigned char sum = 0;
    
    while (length--) {
        sum += *p;      /* Byte access via register */
        p = p + 1;      /* Increment separately */
    }
    return sum;
}

int main() {
    /* Test data setup */
    const int ARRAY_SIZE = 100;
    int int_array[ARRAY_SIZE];
    int int_array2[ARRAY_SIZE];
    char char_buffer[ARRAY_SIZE];
    char char_buffer2[ARRAY_SIZE];
    short short_array[ARRAY_SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3;
        char_buffer[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
    }
    
    /* Call pattern functions to generate RTL sequences */
    int sum1 = sum_array_int(int_array, ARRAY_SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    copy_buffer_char(char_buffer2, char_buffer, ARRAY_SIZE);
    printf("Char buffer copied, first byte: %d\n", char_buffer2[0]);
    
    short result = process_shorts(short_array, ARRAY_SIZE);
    printf("XOR of shorts: %d\n", result);
    
    fill_sequence(int_array2, ARRAY_SIZE, 10);
    printf("Sequence filled, first value: %d\n", int_array2[0]);
    
    int total = sum_and_copy(int_array, int_array2, ARRAY_SIZE / 2);
    printf("Sum and copy total: %d\n", total);
    
    int sum2 = traverse_with_pointer(int_array, ARRAY_SIZE);
    printf("Pointer traversal sum: %d\n", sum2);
    
    unsigned char csum = checksum((unsigned char*)char_buffer, ARRAY_SIZE);
    printf("Checksum: %u\n", (unsigned int)csum);
    
    /* Additional pointer usage in main itself */
    int *ptr = int_array;
    int local_sum = 0;
    
    /* This loop in main should also generate the pattern */
    for (int i = 0; i < 10; i++) {
        local_sum += *ptr;  /* Simple (mem (reg)) */
        ptr = ptr + 1;      /* Separate increment */
    }
    printf("Local sum from main: %d\n", local_sum);
    
    /* Verification */
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += int_array[i];
    }
    
    if (sum1 + sum2 == expected_sum * 2 - local_sum) {
        printf("All computations consistent\n");
    } else {
        printf("Computation mismatch\n");
    }
    
    return 0;
}
