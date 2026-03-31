/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference with post-increment in loop */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += *p;      /* (mem (reg p)) pattern */
        p = p + 1;      /* Subsequent increment - should trigger optimization */
    }
    return sum;
}

/* Pattern 2: Char pointer traversal with restrict */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    while (n-- > 0) {
        *d = *s;        /* Simple (mem (reg)) access */
        d = d + 1;      /* Separate increment */
        s = s + 1;      /* Separate increment */
    }
}

/* Pattern 3: Pointer arithmetic split across statements */
int process_short_data(short *data, int count) {
    short *ptr = data;
    int total = 0;
    
    for (int i = 0; i < count; i++) {
        short val = *ptr;   /* Dereference */
        ptr = ptr + 1;      /* Increment in next statement */
        total += val;
    }
    return total;
}

/* Pattern 4: While loop with pointer post-increment */
void fill_with_value(int *buffer, int size, int value) {
    int *p = buffer;
    int remaining = size;
    
    while (remaining--) {
        *p = value;     /* Store with simple address */
        p = p + 1;      /* Explicit increment */
    }
}

/* Pattern 5: Mixed types and operations */
float average_float(const float *values, int n) {
    const float *ptr = values;
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        float v = *ptr;     /* Load float */
        ptr = ptr + 1;      /* Increment pointer */
        sum += v;
    }
    return (n > 0) ? sum / n : 0.0f;
}

/* Pattern 6: Direct pointer manipulation in main */
void process_direct(int *arr, int n) {
    int *p = arr;
    int *end = arr + n;
    
    while (p < end) {
        /* This should generate (mem (reg p)) */
        *p = (*p) * 2 + 1;
        p = p + 1;  /* Separate increment */
    }
}

/* Pattern 7: Byte-wise checksum with char pointer */
unsigned char checksum(const unsigned char *data, int length) {
    const unsigned char *ptr = data;
    unsigned char sum = 0;
    
    for (int i = 0; i < length; i++) {
        sum ^= *ptr;    /* XOR with dereferenced byte */
        ptr = ptr + 1;  /* Pointer increment */
    }
    return sum;
}

/* Pattern 8: Structure access through pointer */
struct Point {
    int x;
    int y;
};

int sum_points(const struct Point *points, int n) {
    const struct Point *p = points;
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        total += p->x + p->y;  /* Access through pointer */
        p = p + 1;             /* Increment to next struct */
    }
    return total;
}

/* Main function with various test cases */
int main(void) {
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Test 1: Integer array sum */
    int int_arr[100];
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i + 1;
    }
    int sum1 = sum_array_int(int_arr, 100);
    printf("Sum of 1..100 = %d (expected: 5050)\n", sum1);
    
    /* Test 2: Char buffer copy */
    char src[50], dst[50];
    for (int i = 0; i < 50; i++) {
        src[i] = 'A' + (i % 26);
    }
    copy_buffer_char(dst, src, 50);
    dst[49] = '\0';
    printf("Copied string: %s\n", dst);
    
    /* Test 3: Short data processing */
    short short_data[20];
    for (int i = 0; i < 20; i++) {
        short_data[i] = (short)(i * 10);
    }
    int sum3 = process_short_data(short_data, 20);
    printf("Sum of shorts = %d\n", sum3);
    
    /* Test 4: Fill with value */
    int buffer[30];
    fill_with_value(buffer, 30, 42);
    printf("Buffer[0] = %d, Buffer[29] = %d\n", buffer[0], buffer[29]);
    
    /* Test 5: Float average */
    float floats[10];
    for (int i = 0; i < 10; i++) {
        floats[i] = (float)i * 1.5f;
    }
    float avg = average_float(floats, 10);
    printf("Average of floats = %.2f\n", avg);
    
    /* Test 6: Direct processing in main-like pattern */
    int direct_arr[15];
    for (int i = 0; i < 15; i++) {
        direct_arr[i] = i;
    }
    process_direct(direct_arr, 15);
    printf("Processed direct_arr[14] = %d\n", direct_arr[14]);
    
    /* Test 7: Byte checksum */
    unsigned char bytes[25];
    for (int i = 0; i < 25; i++) {
        bytes[i] = (unsigned char)(i * 7);
    }
    unsigned char csum = checksum(bytes, 25);
    printf("Checksum = 0x%02X\n", csum);
    
    /* Test 8: Structure traversal */
    struct Point points[5];
    for (int i = 0; i < 5; i++) {
        points[i].x = i * 2;
        points[i].y = i * 3;
    }
    int point_sum = sum_points(points, 5);
    printf("Sum of points = %d\n", point_sum);
    
    /* Additional pattern in main itself */
    int *main_ptr = int_arr;
    int main_sum = 0;
    for (int i = 0; i < 10; i++) {
        main_sum += *main_ptr;  /* Direct dereference */
        main_ptr = main_ptr + 1; /* Separate increment */
    }
    printf("Main loop sum = %d\n", main_sum);
    
    /* Final verification */
    int final_check = sum1 + sum3 + point_sum + main_sum;
    printf("Final combined result = %d\n", final_check);
    
    if (final_check == (5050 + 1900 + 100 + 55)) {
        printf("All tests passed!\n");
    } else {
        printf("Test mismatch!\n");
    }
    
    return 0;
}
