/* Test file for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Parentheses case: Function pointer typedef */
typedef int (*func_ptr_t)(int, char);

/* Brackets case: Array in struct */
struct GTY(()) ArrayStruct {
    int data[10];
    int (*callbacks[5])(void);
};

/* Braces case: Struct definition with GTY marker */
struct GTY(()) TreeNode {
    struct TreeNode *GTY((skip)) left;
    struct TreeNode *right;
    int values[3];
};

/* Combined: Array of function pointers */
extern int (*GTY((tag("CALLBACKS"))) signal_handlers[10])(int, void*);

/* Nested parentheses in function pointer */
void (*(*complex_callback)(int))(void);

/* Multi-dimensional array with size expression */
extern char buffer[256][(sizeof(int) * 4)];

#endif /* TEST_GTY_H */
