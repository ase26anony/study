#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) patterns */
    while (n-- > 0) {
        sum += *p;      /* Simple dereference: (mem (reg p)) */
        p = p + 1;      /* Separate increment operation */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict qualifier */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    /* Using restrict helps alias analysis */
    while (n-- > 0) {
        *dst = *src;    /* Simple dereference of both pointers */
        dst = dst + 1;  /* Separate increments */
        src = src + 1;
    }
}

/* Pattern 3: Mixed operations in loop - should still trigger */
void fill_buffer_short(short *buf, short value, int n) {
    short *p = buf;
    
    for (int i = 0; i < n; i++) {
        *p = value;     /* Store with simple address */
        p = p + 1;      /* Increment separately */
    }
}

/* Pattern 4: Pointer traversal with explicit temporary */
int traverse_and_sum(int *ptr, int count) {
    int total = 0;
    int *current = ptr;
    
    while (count > 0) {
        int val = *current;  /* Load with (mem (reg current)) */
        total += val;
        current = current + 1;  /* Separate increment */
        count--;
    }
    return total;
}

/* Pattern 5: Nested simple accesses */
void process_pairs(int *restrict a, int *restrict b, int n) {
    for (int i = 0; i < n; i++) {
        int x = *a;     /* First access */
        int y = *b;     /* Second access */
        *a = x + y;     /* Store back */
        a = a + 1;      /* Increment after use */
        b = b + 1;
    }
}

/* Pattern 6: Local pointer with no function arguments */
int sum_local_array(void) {
    int local_arr[16];
    int *p = local_arr;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        local_arr[i] = i;
    }
    
    /* Pointer traversal */
    for (int i = 0; i < 16; i++) {
        sum += *p;      /* Simple dereference */
        p = p + 1;      /* Separate increment */
    }
    return sum;
}

/* Pattern 7: While loop with pointer increment in body */
int find_value(const int *haystack, int needle, int size) {
    const int *p = haystack;
    int pos = 0;
    
    while (pos < size) {
        if (*p == needle) {  /* Simple dereference in condition */
            return pos;
        }
        p = p + 1;      /* Increment in loop body */
        pos++;
    }
    return -1;
}

/* Pattern 8: Multiple dereferences with same base */
void swap_adjacent(int *ptr, int n) {
    for (int i = 0; i < n - 1; i += 2) {
        int temp = *ptr;        /* First load */
        *ptr = *(ptr + 1);      /* Load from offset, store to base */
        ptr = ptr + 1;          /* Increment */
        *ptr = temp;            /* Store to new base */
        ptr = ptr + 1;          /* Increment again */
    }
}

/* Main function with various test cases */
int main(void) {
    int int_array[100];
    char char_buffer[256];
    char char_buffer2[256];
    short short_buffer[50];
    
    int total_sum = 0;
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i;
    }
    
    memset(char_buffer, 'A', sizeof(char_buffer));
    memset(short_buffer, 0, sizeof(short_buffer));
    
    /* Test 1: Sum array with pointer traversal */
    total_sum += sum_array_int(int_array, 100);
    
    /* Test 2: Copy buffer */
    copy_buffer_char(char_buffer2, char_buffer, 256);
    
    /* Test 3: Fill buffer */
    fill_buffer_short(short_buffer, 42, 50);
    
    /* Test 4: Traverse and sum */
    total_sum += traverse_and_sum(int_array, 50);
    
    /* Test 5: Process pairs */
    int pair_a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int pair_b[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    process_pairs(pair_a, pair_b, 10);
    
    /* Test 6: Local array sum */
    total_sum += sum_local_array();
    
    /* Test 7: Find value */
    int pos = find_value(int_array, 42, 100);
    
    /* Test 8: Swap adjacent */
    int swap_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    swap_adjacent(swap_array, 10);
    
    /* Also include pointer traversal in main itself */
    {
        int *ptr = int_array;
        int local_sum = 0;
        
        /* This loop in main should also generate the pattern */
        for (int i = 0; i < 10; i++) {
            local_sum += *ptr;   /* Simple dereference */
            ptr = ptr + 1;       /* Separate increment */
        }
        total_sum += local_sum;
    }
    
    /* Verify the copy worked */
    if (memcmp(char_buffer, char_buffer2, 256) != 0) {
        printf("Error: Copy verification failed\n");
        return 1;
    }
    
    /* Print final checksum */
    printf("Checksum: %d\n", total_sum);
    printf("Found 42 at position: %d\n", pos);
    
    return 0;
}
