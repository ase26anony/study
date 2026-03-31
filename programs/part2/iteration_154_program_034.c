Thread 1: reads g = 10, local = 5
Thread 2: reads g = 10, local = 5
Thread 1: evaluates (10 > 5) = true
Thread 2: evaluates (10 > 5) = true  
Thread 1: sets g = 5
Thread 2: sets g = 5 (again)
// Final g = 5, but the redundant write is problematic
