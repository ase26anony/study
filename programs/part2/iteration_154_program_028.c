Thread 1: Reads g = 0
Thread 2: Reads g = 0
Thread 1: Sets g = 5 (since 0 > 5 is false, but let's say g was initially >5)
Thread 2: Sets g = 5 (based on stale read)
