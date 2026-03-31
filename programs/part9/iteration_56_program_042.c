// To increase pressure further:
void even_higher_pressure() {
    // Add more variables of different types
    double da, db, dc;
    long long la, lb, lc;
    
    // Add more complex data dependencies
    for (...) {
        // Force anti-dependencies
        a = b + c;
        b = a + d;
        c = b + e;
        
        // Add more conditional branches
        if (a % 2) { /* path A */ }
        else if (a % 3) { /* path B */ }
        else { /* path C */ }
    }
}
