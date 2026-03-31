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

/* Array type for use in other structures */
typedef int IntArray[10];

/* Another callback type */
typedef void (*event_handler)(int event_id, void* user_data);

/* Packed struct with attribute */
struct __attribute__((packed, aligned(4))) PackedData {
    char flag;
    int value;
};

/* Deprecated typedef */
typedef int OldInt __attribute__((deprecated));

/* Aligned struct */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
    long long timestamp;
};

/* Extern declarations */
extern struct Point global_point;
extern MyStruct global_mystruct;
extern union Value global_value;

#endif /* TEST_TYPES_H */
