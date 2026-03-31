/* types.h - Header file with declarations for gengtype test */

#ifndef TYPES_H
#define TYPES_H

/* Forward declaration for TYPE_UNDEFINED */
struct Opaque;

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (*comparator)(const void*, const void*);

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
    double z;
};

/* TYPE_UNION */
union Value {
    int i;
    float f;
    double d;
    char* s;
};

/* Array type for use in other structs */
typedef int IntArray[10];

/* Complex pointer type */
typedef volatile const int* volatile* ComplexPtr;

/* GCC attributes for testing */
struct __attribute__((packed, aligned(4))) PackedStruct {
    char a;
    int b;
    short c;
};

/* Deprecated type */
typedef int OldInt __attribute__((deprecated));

/* External declarations */
extern struct Point global_point;
extern MyStruct global_mystruct;
extern union Value global_value;

/* Callback function type */
typedef void (*event_handler)(int event_id, void* user_data);

#endif /* TYPES_H */
