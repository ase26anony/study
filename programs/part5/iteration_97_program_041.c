#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char))(float, double);

/* Nested array declarations with initializers */
int matrix_3d[2][3][4] = {
    {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    },
    {
        {13, 14, 15, 16},
        {17, 18, 19, 20},
        {21, 22, 23, 24}
    }
};

/* Structure with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
        } bits;
        struct {
            long x;
            long y;
        } coords;
        char raw[4];
    } data;
    
    int (*callback)(struct OuterStruct *self, int value);
    int array[2][3];
};

/* Function with complex parameter list containing comments */
void process_data(
    int count,                  /* Number of elements */
    int (*filter)(int, int),    /* Filter function */
    int data[][3]               /* 2D array parameter */
);

#endif /* COMPLEX_TYPES_H */
