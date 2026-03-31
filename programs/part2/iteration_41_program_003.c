GPU/Accelerator
├── Team 0: processes j = 0..(N/num_teams - 1)
├── Team 1: processes j = (N/num_teams)..(2N/num_teams - 1)
└── ...
    Each team: vectorizes inner loop with SIMD (up to 16 elements at once)
