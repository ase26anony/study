Thread 1: reads g = 10, local = 5 → g > local is true
Thread 2: reads g = 10, local = 5 → g > local is true
Thread 1: sets g = 5
Thread 2: sets g = 5 (redundant, but not necessarily harmful in this case)
// However, if there were other operations depending on g's value,
// the interleaving could cause logic errors
