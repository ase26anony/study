#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char))(void);
float (*(*(*nested_func_ptr[3])(int))(float))(double);

/* Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][4] = {
    { /* First 2D slice */
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    },
    { /* Second 2D slice */
        {13, 14, 15, 16},
        {17, 18, 19, 20},
        {21, 22, 23, 24}
    }
};

/* Structures with nested anonymous structures and bit-fields */
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
    
    int (*callback)(struct OuterStruct *self, int param);
    int matrix[2][2];
};

/* Union with deeply nested structures */
union MegaUnion {
    struct {
        struct {
            int a;
            struct {
                char x;
                char y;
            } point;
        } level1;
        union {
            int i;
            float f;
        } level2;
    } nested;
    
    void (*func_ptrs[3])(int, int);
};

#endif /* COMPLEX_TYPES_H */
