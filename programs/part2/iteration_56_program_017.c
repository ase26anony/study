/* test_types.h - Header file for multi-file type declarations */
#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
void undefined_function(void); /* void type */

/* TYPE_STRUCT: Plain C structure */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct {
    int data;
    char *name;
} MyStruct;

/* TYPE_UNION: Union declaration */
union Data {
    int i;
    float f;
    double d;
    char *str;
};

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long data[2];
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Callback)(int, void *);

/* External declarations for multi-file testing */
extern struct Point global_point;
extern MyStruct global_mystruct;

#endif /* TEST_TYPES_H */
