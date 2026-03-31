Thread 1: reads g = 10
Thread 2: reads g = 10
Thread 1: 10 > 5 = true, enters if-block
Thread 2: 10 > 5 = true, enters if-block  
Thread 1: sets g = 5
Thread 2: sets g = 5 (again)
