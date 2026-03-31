// main.cpp - C++ main program targeting specific DWARF attributes
#include <iostream>
#include <thread>
#include <atomic>

// For DW_AT_explicit
class ExplicitConstructor {
    int value;
public:
    explicit ExplicitConstructor(int v) : value(v) {}
    explicit operator bool() const { return value != 0; }
};

// For DW_AT_mutable
class WithMutable {
    mutable int counter;
    int value;
public:
    WithMutable(int v) : counter(0), value(v) {}
    int get() const { 
        ++counter;  // mutable member modified in const method
        return value; 
    }
};

// For DW_AT_location and DW_AT_segment
__attribute__((section(".mysection"))) 
int custom_section_var = 42;

thread_local int tls_var = 100;  // May generate location lists

// For DW_AT_prototyped
extern "C" {
    void fortran_subroutine(char* str, int* len);
    void ada_procedure();
    void cobol_program();
}

// Complex template to force full debug info
template<typename T>
class Container {
    T* data;
    size_t size;
public:
    explicit Container(size_t n) : data(new T[n]), size(n) {}
    ~Container() { delete[] data; }
    
    mutable std::atomic<int> access_count;  // mutable + atomic
    
    T& operator[](size_t idx) {
        ++access_count;
        return data[idx];
    }
};

// Force location information with inline assembly
void use_with_asm(int& x) {
    asm volatile ("# %0" : : "r" (x));
}

int main() {
    // Use explicit constructor
    ExplicitConstructor ec(10);
    if (ec) {
        std::cout << "Explicit constructor used\n";
    }
    
    // Use mutable member
    WithMutable wm(20);
    std::cout << "Mutable access: " << wm.get() << "\n";
    
    // Use custom section variable
    custom_section_var = 100;
    use_with_asm(custom_section_var);
    
    // Use TLS variable
    tls_var = 200;
    
    // Use template with mutable atomic
    Container<int> c(5);
    c[0] = 1;
    
    // Call Fortran subroutine
    char str[] = "Hello";
    int len = 5;
    fortran_subroutine(str, &len);
    
    // Prevent dead code elimination
    volatile int sink = 0;
    sink = ec ? 1 : 0;
    sink = wm.get();
    sink = custom_section_var;
    sink = tls_var;
    sink = c[0];
    
    std::cout << "Program completed\n";
    return 0;
}
