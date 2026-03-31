#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;           /* Forward declaration */
union IncompleteUnion;             /* Forward declaration */
typedef struct OpaqueType OpaqueType; /* Opaque typedef */

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

struct NestedContainer {
    struct Point points[10];
    int count;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[50];
} Employee;

typedef struct Node {
    int data;
    struct Node* next;  /* Self-referential pointer */
} ListNode;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char str[20];
};

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long data[4];
};

/* Complex callback type for cross-file use */
typedef int (*FileCallback)(const char*, int);

/* External declarations for multi-file testing */
extern struct Point global_point;
extern Employee global_employee;
extern void process_types(void);

#endif /* TEST_TYPES_H */
