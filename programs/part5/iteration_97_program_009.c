#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Complex type declarations with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char**))(float, double);

/* Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][2] = {
    {{1, 2}, {3, 4}, {5, 6}},
    {{7, 8}, {9, 10}, {11, 12}}
};

/* Structures/unions with nested anonymous structures and bit-fields */
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
    int d[2];
    struct {
        struct {
            int nested_a;
            int nested_b;
        } inner;
        float f;
    } extra;
};

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);
registry_fn find_registry(const char* name);

/* Function with comments between parameters */
void process_data(
    int count,          /* number of items */
    char** items,       /* array of strings */
    void (*callback)(   /* completion callback */
        int status,     /* 0=success, other=error */
        void* data      /* user data */
    )
);

#endif /* TEST_TYPES_H */
