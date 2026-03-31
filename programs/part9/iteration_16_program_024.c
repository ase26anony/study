#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparator)(const void*, const void*);

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char* name;  /* TYPE_STRING */
} MyStruct;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
    volatile const int* volatile* complex_ptr;  /* Complex pointer with qualifiers */
};

/* TYPE_UNION */
union Value {
    int i;
    float f;
    double d;
    char c;
};

/* Array type */
typedef int IntArray[10];

/* Complex nested type */
struct ComplexContainer {
    union Value values[5];  /* Array of unions */
    struct Point* points;   /* Pointer to struct */
    comparator cmp_func;    /* Callback */
    volatile int flags;
};

/* GCC attributes */
struct __attribute__((packed, aligned(4))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((deprecated));

/* Another callback type */
typedef void (*event_handler)(struct ComplexContainer*, int);

/* External declarations */
extern struct Point global_point;
extern union Value global_value;
extern MyStruct global_mystruct;

#endif /* TEST_TYPES_H */
