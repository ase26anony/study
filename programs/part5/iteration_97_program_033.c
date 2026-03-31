#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char))(float, double);

/* Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][4] = {
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
    
    int matrix[2][2];
    
    struct {
        struct {
            int depth;
            void (*callback)(int, char);
        } inner;
        float values[3];
    } nested;
};

/* Union with deeply nested structures */
union ComplexUnion {
    struct {
        int (*compare)(int, int);
        char name[20];
    } func_data;
    
    struct {
        struct {
            int x:5;
            int y:5;
            int z:6;
        } point;
        int arr[2][2][2];
    } spatial_data;
    
    long long raw_value;
};

#endif /* COMPLEX_TYPES_H */
