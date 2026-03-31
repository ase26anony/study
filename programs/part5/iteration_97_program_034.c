#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test case 1: Function pointer types with nested parameter lists */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char**))(float, double);

/* Test case 2: Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][4] = {
    { /* Layer 0 */
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    },
    { /* Layer 1 */
        {13, 14, 15, 16},
        {17, 18, 19, 20},
        {21, 22, 23, 24}
    }
};

/* Test case 3: Structures with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;    /* Comment inside bit-field */
            int b:4;
            int c:8;
        };
        struct {
            unsigned int x:16;
        };
        char raw[4];
    } data;
    
    struct {
        int (*callback)(int, int);
        void* ptr;
    } nested;
    
    int d[2];
};

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);

/* Complex union with nested structs */
union MegaUnion {
    struct {
        int (*func1)(int, int);
        char arr[10];
    } s1;
    struct {
        void (**func_table)(void);
        double matrix[2][2];
    } s2;
};

#endif /* COMPLEX_TYPES_H */
