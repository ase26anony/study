/* test-input.c - Example input for gengtype */
#include <stdio.h>

struct my_struct {
    int x;
    char *name;
    struct my_struct *next;
};

typedef struct my_struct my_struct_t;

union my_union {
    int ival;
    double dval;
    char *sval;
};
