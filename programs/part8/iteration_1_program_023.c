/* gengtype_coverage_test.c - ISO C99 compliant test generator for gengtype parser */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Test 1: Complex balanced constructs with all delimiter types */
static const char *test1_content =
    "/* Test file for gengtype parser coverage */\n"
    "\n"
    "/* Nested parentheses in function pointers */\n"
    "%typedef int (*complex_func_t)(int (*callback)(int, char**), void*);\n"
    "\n"
    "/* Multi-dimensional arrays with nested structs */\n"
    "%struct Matrix {\n"
    "    double data[10][20];\n"
    "    struct {\n"
    "        int rows;\n"
    "        int cols;\n"
    "    } dims;\n"
    "};\n"
    "\n"
    "/* Union with nested arrays and function pointers */\n"
    "%union Value {\n"
    "    int i;\n"
    "    double d;\n"
    "    char *str;\n"
    "    struct {\n"
    "        int (*compare)(const void*, const void*);\n"
    "        void (*free)(void*);\n"
    "    } ops;\n"
    "    int array[5][(2 + 3) * 4];  /* Parentheses in array dimension */\n"
    "};\n"
    "\n"
    "/* GC roots with complex type */\n"
    "GC roots {\n"
    "    struct Matrix *global_matrix;\n"
    "    %union Value *values[100];\n"
    "    int (*handlers[10])(int, char**);\n"
    "}\n"
    "\n"
    "/* Enum with bitfield calculations */\n"
    "%enum Flags {\n"
    "    FLAG_A = 1 << 0,\n"
    "    FLAG_B = 1 << 1,\n"
    "    FLAG_C = (1 << 2) | (1 << 3)  /* Parentheses in enum value */\n"
    "};\n"
    "\n"
    "/* Macro with balanced delimiters */\n"
    "#define ARRAY_TYPE(T) struct { T data[(sizeof(T) > 8) ? 10 : 20]; }\n"
    "#define FUNC_PTR(RET, ARGS) RET (*)(ARGS)\n"
    "\n"
    "/* Apply macros */\n"
    "ARRAY_TYPE(int) int_array;\n"
    "FUNC_PTR(int, (int, char**)) parser_func;\n"
    "\n"
    "/* Nested anonymous struct in union */\n"
    "%union Container {\n"
    "    struct {\n"
    "        struct {\n"
    "            int x, y;\n"
    "        } point;\n"
    "        int data[10];\n"
    "    } s;\n"
    "    union {\n"
    "        void *ptr;\n"
    "        long long ll;\n"
    "    } u;\n"
    "};\n";

/* Test 2: Unbalanced delimiters for error handling coverage */
static const char *test2_content =
    "/* Test with unbalanced delimiters */\n"
    "\n"
    "%struct Unbalanced1 {\n"
    "    int data[10;  /* Missing closing bracket */\n"
    "    char *name;\n"
    "};\n"
    "\n"
    "%union Unbalanced2 {\n"
    "    int x;\n"
    "    double y\n"  /* Missing semicolon and closing brace */
    "\n"
    "/* Function pointer with unbalanced parens */\n"
    "%typedef void (*bad_func)(int, char*;  /* Missing closing paren */\n"
    "\n"
    "GC roots {\n"
    "    int *ptr;\n"  /* Missing closing brace for GC roots */
    "\n"
    "/* Comment with unbalanced delimiter inside */\n"
    "/* This comment has a { without closing */\n"
    "int var;\n";

/* Test 3: Comments and macros interleaving with delimiters */
static const char *test3_content =
    "/* Test: Delimiters inside comments and macros */\n"
    "\n"
    "// Line comment with { braces } and (parentheses)\n"
    "// And [brackets] too\n"
    "\n"
    "/* Block comment with \n"
    "   nested { struct { int x; } s; }\n"
    "   and function pointer: int (*func)(int[10]);\n"
    "   closing here */\n"
    "\n"
    "#define NESTED_PAREN(x) (((x) + 1) * 2)\n"
    "#define ARRAY_DECL(type, size) type name[size]\n"
    "#define STRUCT_WITH_UNION \\\n"
    "    struct { \\\n"
    "        union { \\\n"
    "            int i; \\\n"
    "            double d; \\\n"
    "        } u; \\\n"
    "    }\n"
    "\n"
    "/* Use the macros with actual declarations */\n"
    "%struct CommentedStruct {\n"
    "    int value NESTED_PAREN(5);  /* Expands to (((5) + 1) * 2) */\n"
    "    ARRAY_DECL(char, 20) buffer;\n"
    "    STRUCT_WITH_UNION data;\n"
    "};\n"
    "\n"
    "/* GC roots between comments */\n"
    "// Line comment before GC\n"
    "GC roots {\n"
    "    %struct CommentedStruct *cs;\n"
    "}\n"
    "/* Block comment after GC */\n"
    "\n"
    "/* Function pointer type with complex signature */\n"
    "%typedef int (*(*signal_handler_t)[5])(int, ...);\n"
    "\n"
    "/* Array of function pointers */\n"
    "int (*(*func_array[10]))(double, char**);\n";

/* Test 4: Multiple top-level declarations for repeated parsing */
static const char *test4_content =
    "/* Multiple type declarations to exercise parser loop */\n"
    "\n"
    "%typedef unsigned int uint32_t;\n"
    "%typedef long long int64_t;\n"
    "\n"
    "%struct Node {\n"
    "    int data;\n"
    "    struct Node *next;\n"
    "    struct Node *prev;\n"
    "};\n"
    "\n"
    "%union Flexible {\n"
    "    int i;\n"
    "    float f;\n"
    "    char c[4];\n"
    "    void *p;\n"
    "};\n"
    "\n"
    "%enum Color {\n"
    "    RED,\n"
    "    GREEN,\n"
    "    BLUE,\n"
    "    ALPHA = 255\n"
    "};\n"
    "\n"
    "GC roots {\n"
    "    %struct Node *list_head;\n"
    "    %union Flexible flex_array[50];\n"
    "    %enum Color *current_color;\n"
    "}\n"
    "\n"
    "%struct Tree {\n"
    "    int value;\n"
    "    %struct Tree *left;\n"
    "    %struct Tree *right;\n"
    "    %union Flexible data;\n"
    "};\n"
    "\n"
    "%typedef %struct Tree *TreePtr;\n"
    "%typedef int (*Comparator)(TreePtr, TreePtr);\n"
    "\n"
    "GC roots {\n"
    "    TreePtr root;\n"
    "    Comparator cmp_func;\n"
    "}\n"
    "\n"
    "/* Complex nested type */\n"
    "%struct Outer {\n"
    "    struct {\n"
    "        int x[(2 + 3) * 4];\n"
    "        struct {\n"
    "            char *name;\n"
    "            int id;\n"
    "        } info;\n"
    "    } inner;\n"
    "    union {\n"
    "        void (*func)(int);\n"
    "        int (*methods[5])(void);\n"
    "    } ops;\n"
    "};\n";

/* Write test content to file */
static int write_test_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Simple parser simulation to directly test the switch logic */
static void simulate_parser_logic(const char *content) {
    const char *p = content;
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    
    while (*p) {
        switch (*p) {
            case '(':
                paren_depth++;
                /* Simulate consume_balanced logic */
                while (*p && paren_depth > 0) {
                    p++;
                    if (*p == '(') paren_depth++;
                    else if (*p == ')') paren_depth--;
                }
                break;
                
            case '[':
                bracket_depth++;
                /* Simulate consume_balanced logic */
                while (*p && bracket_depth > 0) {
                    p++;
                    if (*p == '[') bracket_depth++;
                    else if (*p == ']') bracket_depth--;
                }
                break;
                
            case '{':
                brace_depth++;
                /* Simulate consume_balanced logic */
                while (*p && brace_depth > 0) {
                    p++;
                    if (*p == '{') brace_depth++;
                    else if (*p == '}') brace_depth--;
                }
                break;
                
            case '/':
                /* Skip comments */
                if (p[1] == '/') {
                    while (*p && *p != '\n') p++;
                } else if (p[1] == '*') {
                    p += 2;
                    while (*p && !(*p == '*' && p[1] == '/')) p++;
                    if (*p) p += 2;
                }
                break;
                
            case '\'':
            case '"':
                /* Skip character and string literals */
                {
                    char quote = *p;
                    p++;
                    while (*p && *p != quote) {
                        if (*p == '\\' && p[1]) p += 2;
                        else p++;
                    }
                    if (*p == quote) p++;
                }
                break;
                
            default:
                p++;
                break;
        }
        
        if (*p) p++;
    }
}

/* Main test driver */
int main(void) {
    const char *test_files[] = {
        "test1_balanced.gt",
        "test2_unbalanced.gt", 
        "test3_comments.gt",
        "test4_multiple.gt"
    };
    
    const char *test_contents[] = {
        test1_content,
        test2_content,
        test3_content,
        test4_content
    };
    
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);
    int i;
    
    printf("Generating gengtype test files for parser coverage...\n");
    
    /* Create test directory */
    mkdir("gtype_tests", 0755);
    
    /* Write test files */
    for (i = 0; i < num_tests; i++) {
        char path[256];
        snprintf(path, sizeof(path), "gtype_tests/%s", test_files[i]);
        
        printf("Creating %s\n", path);
        if (!write_test_file(path, test_contents[i])) {
            fprintf(stderr, "Failed to write %s\n", path);
            return 1;
        }
        
        /* Simulate parser logic for coverage analysis */
        printf("Simulating parser logic for %s\n", test_files[i]);
        simulate_parser_logic(test_contents[i]);
    }
    
    /* Create a comprehensive combined test */
    const char *combined_content = 
        "/* Combined test with all patterns */\n"
        "\n"
        "/* From test1 */\n"
        "%typedef int (*func_t)(int (*)(int, char**), void*);\n"
        "%struct Combined { double m[2][3]; struct { int r, c; } d; };\n"
        "\n"
        "/* From test2 - include some unbalanced for error handling */\n"
        "// int bad[5;  // Would trigger error\n"
        "\n"
        "/* From test3 */\n"
        "#define COMPLEX_ARRAY(T) T arr[(sizeof(T) + 3) & ~3]\n"
        "COMPLEX_ARRAY(int) ints;\n"
        "\n"
        "/* From test4 */\n"
        "GC roots {\n"
        "    %struct Combined *matrix;\n"
        "    func_t handlers[5];\n"
        "}\n"
        "\n"
        "/* Additional complex nesting */\n"
        "%union Ultimate {\n"
        "    struct {\n"
        "        int (*compare)(const void*, const void*);\n"
        "        void (*destructor)(void*);\n"
        "    } ops;\n"
        "    struct {\n"
        "        int data[10][(2 * 3) + 4];\n"
        "        char *name;\n"
    "    } info;\n"
        "    union {\n"
        "        long long ll;\n"
        "        double d;\n"
        "    } value;\n"
        "};\n";
    
    printf("Creating combined test file\n");
    if (!write_test_file("gtype_tests/combined.gt", combined_content)) {
        fprintf(stderr, "Failed to write combined test\n");
        return 1;
    }
    
    /* Simulate parser on combined content */
    simulate_parser_logic(combined_content);
    
    printf("\nTest files generated in gtype_tests/ directory:\n");
    for (i = 0; i < num_tests; i++) {
        printf("  %s\n", test_files[i]);
    }
    printf("  combined.gt\n");
    
    printf("\nTo run actual gengtype parser:\n");
    printf("  gcc -o gengtype gengtype-parse.cc gengtype.cc gengtype-state.cc -I.\n");
    printf("  ./gengtype -p gtype_tests/combined.gt\n");
    
    printf("\nFor coverage analysis:\n");
    printf("  g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
    printf("      -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("  ./gengtype-instr -p gtype_tests/combined.gt\n");
    printf("  gcov gengtype-parse.cc\n");
    
    return 0;
}
