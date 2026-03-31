#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char))(float, double);

/* Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][2] = {
    {
        {1, 2},
        {3, 4},
        {5, 6}
    },
    {
        {7, 8},
        {9, 10},
        {11, 12}
    }
};

/* Structures with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
        };
        struct {
            unsigned int x:16;
        };
        char raw[4];
    } nested_union;
    
    struct {
        int (*callback)(int, int);
        char data[10];
    } inner_struct;
    
    int d[2];
};

/* Union with deeply nested structures */
union ComplexUnion {
    struct {
        int (*func_ptr_array[3])(int, int);
        struct {
            char a;
            short b;
            long c;
        } nested;
    } s;
    long long data[4];
};

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);
typedef registry_fn (*manager_fn)(int, registry_fn);

/* Function returning function pointer to function returning function pointer */
int (*(*(*ultimate_func_ptr)(int))(float))(char);

#endif /* COMPLEX_TYPES_H */
