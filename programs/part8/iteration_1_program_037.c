/* gengtype_coverage_test.c - ISO C99 compliant test generator for gengtype parser */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Test 1: Complex balanced constructs with all delimiter types */
static const char *test1_content =
    "/* Test 1: Complex nested structures with all delimiter types */\n"
    "%struct ComplexStruct {\n"
    "    int (*compare)(const void *, const void *);  /* Function pointer */\n"
    "    union {\n"
    "        struct {\n"
    "            int matrix[3][3];\n"
    "            void (*callback)(int (*)(int), int);  /* Nested function pointers */\n"
    "        } nested;\n"
    "        char *(*string_ops[5])(char *, const char *);  /* Array of function pointers */\n"
    "    } u;\n"
    "    struct {\n"
    "        int (*transform)(int[10], void (*)(int));\n"
    "        double (*compute)(double (*)(double, double), double, double);\n"
    "    } ops;\n"
    "};\n"
    "\n"
    "/* Array type with complex initialization */\n"
    "%typedef struct {\n"
    "    int values[(10 + 5) * 2];  /* Parentheses in array size */\n"
    "    struct Node *(*find)(struct Node *[100], int (*pred)(int));\n"
    "} ComplexArrayType;\n"
    "\n"
    "/* Union with nested arrays and function pointers */\n"
    "%union TaggedUnion {\n"
    "    struct {\n"
    "        int (*methods[3])(void);\n"
    "        char data[{ /* Nested braces in comment */ }];\n"
    "    } s;\n"
    "    void *(*allocator)(size_t (*)(size_t));\n"
    "};\n";

/* Test 2: Edge cases with unbalanced delimiters */
static const char *test2_content =
    "/* Test 2: Unbalanced delimiter edge cases */\n"
    "%struct UnbalancedTest {\n"
    "    int missing_paren = (5 + (3 * 2);  /* Missing closing paren */\n"
    "    double matrix[3][3;  /* Missing closing bracket */\n"
    "    struct {\n"
    "        int x;\n"
    "        char y;\n"
    "    ;  /* Missing closing brace */\n"
    "\n"
    "    /* But also include valid balanced ones after errors */\n"
    "    void (*valid_func)(int, char);\n"
    "    int valid_array[10];\n"
    "};\n"
    "\n"
    "/* Another unbalanced case */\n"
    "%typedef struct BadTypedef {\n"
    "    int (*func[5](int);  /* Mixed up brackets and parens */\n"
    "    char data{10];  /* Wrong delimiter pairing */\n"
    "} BadTypedef;\n";

/* Test 3: Comments and macros interleaving with delimiters */
static const char *test3_content =
    "/* Test 3: Delimiters inside comments and macros */\n"
    "#define ARRAY_TYPE(T) struct { T data[10]; (void)0; }\n"
    "#define FUNC_PTR(RET, ARGS) RET (*)(ARGS)\n"
    "#define NESTED_MACRO(X) { int y = (X + 1); }\n"
    "\n"
    "/* Comment with delimiters: int x = {1, 2, 3}; */\n"
    "// Line comment with parens: void func(int (*)(int))\n"
    "%struct MacroStruct {\n"
    "    ARRAY_TYPE(int) int_array;  /* Expands to struct with braces */\n"
    "    FUNC_PTR(int, (int, char)) func_ptr;  /* Nested parentheses */\n"
    "    /* Inside block comment: int matrix[3][3] = {{1,2,3},{4,5,6}}; */\n"
    "    struct {\n"
    "        // Line comment: void (*)(int[][5])\n"
    "        int (*processor)(int (*)[5], void (*callback)(int));\n"
    "    } nested;\n"
    "};\n"
    "\n"
    "/* Macro that generates balanced constructs */\n"
    "#define CREATE_UNION(TYPE) %union { TYPE data; void *ptr; }\n"
    "CREATE_UNION(int) IntUnion;\n"
    "CREATE_UNION(struct { int x; double y; }) ComplexUnion;\n";

/* Test 4: GT-specific annotations with embedded delimiters */
static const char *test4_content =
    "/* Test 4: GT-specific annotations */\n"
    "GC roots {\n"
    "    struct RootStruct *global_root;\n"
    "    int (*global_funcs[5])(void);\n"
    "    union {\n"
    "        struct Node *tree_root;\n"
    "        void (*cleanup)(struct RootStruct *);\n"
    "    } u;\n"
    "}\n"
    "\n"
    "%struct GcAnnotated {\n"
    "    int *%gc_ptr% data;  /* Annotation with percent signs */\n"
    "    struct {\n"
    "        void (*%gc_callback%)(int *%gc_ptr%);\n"
    "        int %gc_array%[10];\n"
    "    } nested;\n"
    "};\n"
    "\n"
    "%union GcUnion {\n"
    "    struct {\n"
    "        int (*compare)(const void *[5], int (*)(int));\n"
    "        char buffer[{256}];  /* Braces in array size (GCC extension) */\n"
    "    } s;\n"
    "    void *%gc_handle% ptr;\n"
    "};\n"
    "\n"
    "/* Typedef with function pointer array */\n"
    "%typedef int (*OperationFuncs[3])(int (*)(int, int), double);\n";

/* Test 5: Multiple top-level declarations for repeated parsing */
static const char *test5_content =
    "/* Test 5: Multiple varied declarations */\n"
    "%enum Color { RED, GREEN, BLUE };\n"
    "\n"
    "%struct First {\n"
    "    int (*func1)(int[10]);\n"
    "    double matrix[2][2];\n"
    "};\n"
    "\n"
    "%union Second {\n"
    "    struct {\n"
    "        void (*action)(void);\n"
    "        int values[5];\n"
    "    } a;\n"
    "    char *(*string_op)(char *, int (*)(char));\n"
    "};\n"
    "\n"
    "%typedef struct Third {\n"
    "    union {\n"
    "        int (*math_op[2])(int, int);\n"
    "        struct {\n"
    "            void (*initialize)(int (*)[3]);\n"
    "            char data[100];\n"
    "        } init;\n"
    "    } u;\n"
    "} ThirdType;\n"
    "\n"
    "%struct Fourth {\n"
    "    /* Complex nested function pointer */\n"
    "    int (*(*get_callback)(void))(int (*)(int), int);\n"
    "    /* Array with computed size */\n"
    "    float samples[(2 * 5) + 3];\n"
    "};\n"
    "\n"
    "GC roots {\n"
    "    struct Fourth *root_fourth;\n"
    "    ThirdType *root_third;\n"
    "}\n";

/* Combined test with all patterns */
static const char *combined_test_content =
    "/* COMBINED TEST: All patterns together */\n"
    "\n"
    "/* 1. Balanced constructs */\n"
    "%struct MasterStruct {\n"
    "    /* Parentheses: function pointers */\n"
    "    void (*simple_func)(int);\n"
    "    int (*(*complex_func)(int (*)(int)))(int, int);\n"
    "    \n"
    "    /* Brackets: arrays */\n"
    "    int simple_array[10];\n"
    "    void (*func_array[5])(int[3][3]);\n"
    "    \n"
    "    /* Braces: nested structures */\n"
    "    struct {\n"
    "        union {\n"
    "            int x;\n"
    "            double y;\n"
    "        } u;\n"
    "        struct {\n"
    "            int (*nested_func)(void);\n"
    "        } deeper;\n"
    "    } nested;\n"
    "};\n"
    "\n"
    "/* 2. Some unbalanced cases for error handling */\n"
    "%struct ErrorTest {\n"
    "    int bad1 = (5 + (3 * 2);  /* Missing ) */\n"
    "    char bad2[10;  /* Missing ] */\n"
    "    struct { int a; ;  /* Missing } */\n"
    "};\n"
    "\n"
    "/* 3. With comments and macros */\n"
    "// Line comment with: int x = {1, 2}\n"
    "#define WRAP(T) struct { T wrapped; }\n"
    "%typedef WRAP(int (*)(int)) FuncWrapper;\n"
    "\n"
    "/* 4. GT annotations */\n"
    "GC roots {\n"
    "    MasterStruct *root;\n"
    "    void (*root_funcs[3])(int);\n"
    "}\n"
    "\n"
    "/* 5. Multiple declarations */\n"
    "%union FinalUnion {\n"
    "    int (*final_func)(int (*[5])(int));\n"
    "    struct { char data[100]; } final_struct;\n"
    "};\n";

/* Parser simulation to directly test the switch logic */
static void simulate_parser(const char *input) {
    const char *p = input;
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    
    while (*p) {
        switch (*p) {
            case '(':
                paren_depth++;
                /* Simulate consume_balanced('(', ')') */
                {
                    const char *q = p + 1;
                    int depth = 1;
                    while (*q && depth > 0) {
                        if (*q == '(') depth++;
                        else if (*q == ')') depth--;
                        q++;
                    }
                    p = q - 1;
                }
                break;
                
            case '[':
                bracket_depth++;
                /* Simulate consume_balanced('[', ']') */
                {
                    const char *q = p + 1;
                    int depth = 1;
                    while (*q && depth > 0) {
                        if (*q == '[') depth++;
                        else if (*q == ']') depth--;
                        q++;
                    }
                    p = q - 1;
                }
                break;
                
            case '{':
                brace_depth++;
                /* Simulate consume_balanced('{', '}') */
                {
                    const char *q = p + 1;
                    int depth = 1;
                    while (*q && depth > 0) {
                        if (*q == '{') depth++;
                        else if (*q == '}') depth--;
                        q++;
                    }
                    p = q - 1;
                }
                break;
                
            default:
                /* advance() simulation */
                break;
        }
        p++;
    }
    
    printf("Parser simulation complete. Depths: paren=%d, bracket=%d, brace=%d\n",
           paren_depth, bracket_depth, brace_depth);
}

/* Write test content to file */
static int write_test_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    
    size_t written = fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return written == strlen(content);
}

/* Main test driver */
int main(void) {
    const char *test_files[] = {
        "test1_balanced.gt",
        "test2_unbalanced.gt",
        "test3_macros.gt",
        "test4_annotations.gt",
        "test5_multiple.gt",
        "combined_test.gt"
    };
    
    const char *test_contents[] = {
        test1_content,
        test2_content,
        test3_content,
        test4_content,
        test5_content,
        combined_test_content
    };
    
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);
    int success_count = 0;
    
    printf("Generating gengtype test files...\n");
    
    /* Write all test files */
    for (int i = 0; i < num_tests; i++) {
        if (write_test_file(test_files[i], test_contents[i])) {
            printf("  Created: %s\n", test_files[i]);
            success_count++;
            
            /* Simulate parsing on the content */
            printf("  Simulating parser on %s...\n", test_files[i]);
            simulate_parser(test_contents[i]);
        } else {
            printf("  Failed to create: %s\n", test_files[i]);
        }
    }
    
    printf("\nGenerated %d/%d test files successfully.\n", success_count, num_tests);
    printf("\nTo run actual gengtype on these files:\n");
    printf("  g++ -O2 -g -I. -I../../include -o gengtype gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("  ./gengtype -p combined_test.gt\n");
    printf("\nFor coverage analysis:\n");
    printf("  g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
    printf("      -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("  ./gengtype-instr -p combined_test.gt\n");
    printf("  gcov gengtype-parse.cc\n");
    
    /* Cleanup - in real test, you might want to keep the files */
    for (int i = 0; i < num_tests; i++) {
        remove(test_files[i]);
    }
    
    return success_count == num_tests ? 0 : 1;
}
