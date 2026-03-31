```cpp
// test_parser_errors.cc
// Compile with: g++ -std=c++11 -fsyntax-only -fdiagnostics-parseable-fixits test_parser_errors.cc
// Or with: g++ -std=c++11 -O1 -fno-exceptions test_parser_errors.cc
// Or with: g++ -std=c++11 -O0 -fpermissive -Wno-error test_parser_errors.cc

#include <iostream>

// Valid code to establish normal parsing state
void valid_function() {
    int x = 42;
    std::cout << "Valid code" << std::endl;
}

// 1. RT_EXTERN: Expected 'extern' in linkage specification
void test_extern_error() {
    // Valid code first
    int a = 1;
    
    // Missing 'extern' before linkage specifier
    "C" {
        void some_function();
    }
}

// 2. RT_STATIC_ASSERT: Expected 'static_assert'
void test_static_assert_error() {
    // Valid code
    double y = 3.14;
    
    // Missing 'static_assert' keyword
    (sizeof(int) == 4, "int must be 4 bytes");
}

// 3. RT_DECLTYPE: Expected 'decltype' in trailing return type
void test_decltype_error() {
    // Valid code
    char c = 'A';
    
    // Missing 'decltype' in trailing return type
    auto foo() -> (c);
}

// 4. RT_OPERATOR: Expected 'operator' in conversion function
class TestClass {
public:
    // Valid member
    int value;
    
    // Missing 'operator' for conversion function
    int() const;
    
    // Valid member function
    void normal_func() {}
};

// 5. RT_CLASS: Expected 'class' in class definition
void test_class_error() {
    // Valid code
    float f = 1.5f;
    
    // Missing 'class' keyword
    MyClass {
    public:
        int x;
    };
}

// 6. RT_TEMPLATE: Expected 'template' in template declaration
void test_template_error() {
    // Valid code
    bool flag = true;
}

// Template missing 'template' keyword - at global scope for ambiguity
<typename T> void template_func() {}

// 7. RT_NAMESPACE: Expected 'namespace' in namespace definition
void test_namespace_error() {
    // Valid code
    short s = 100;
}

// Missing 'namespace' keyword
MyNamespace {
    int variable;
}

// 8. RT_USING: Expected 'using' in using directive
void test_using_error() {
    // Valid code
    long l = 1000L;
    
    // Missing 'using' keyword
    namespace std;
    
    // Another attempt with using-declaration
    std::cout;
}

// 9. RT_ASM: Expected 'asm' in inline assembly
void test_asm_error() {
    // Valid code
    unsigned u = 500;
    
    // Missing 'asm' keyword
    volatile ("nop");
}

// 10. RT_TRY: Expected 'try' in try block
void test_try_error() {
    // Valid code
    int arr[5] = {1, 2, 3, 4, 5};
    
    // Missing 'try' keyword
    {
        throw 42;
    } catch (...) {
        // Handle exception
    }
}

// Additional valid code to ensure parser continues
namespace ValidNamespace {
    template<typename T>
    class ValidTemplate {
    public:
        T data;
        
        ValidTemplate(T d) : data(d) {}
        
        operator T() const { return data; }
    };
}

// Valid main function to keep overall structure
int main() {
    std::cout << "This program triggers parser errors for coverage testing" << std::endl;
    
    // Call some valid functions
    valid_function();
    
    // The following would call error functions if they were valid
    // test_extern_error();  // Commented out because they have errors
    
    return 0;
}

// Additional global scope errors to trigger different parsing contexts

// Another RT_TEMPLATE case at global scope
<typename U> struct AnotherTemplateStruct;

// Another RT_CLASS case
AnotherClass {
    void method() {}
};

// Another RT_OPERATOR case in a different class
struct AnotherTestStruct {
    // Missing 'operator'
    double() const volatile;
};

// RT_DECLTYPE in using alias (C++11 feature)
using MyAlias = (int);  // Missing 'decltype'

// RT_STATIC_ASSERT at class scope
class ClassWithStaticAssert {
    // Valid member
    int member;
    
    // Missing 'static_assert'
    (sizeof(member) == 4);
};

// RT_EXTERN for variable declaration
"C" int external_var;  // Missing 'extern'

// RT_ASM with different syntax
__volatile__ ("mov eax, ebx");  // Missing 'asm'

// RT_TRY with function-try-block
void function_with_try() 
    { throw 0; } catch(...) {}  // Missing 'try' after parameter list
```
