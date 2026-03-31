Device (e.g., GPU)
├── Multiple teams created
│   ├── Team 0: Processes j = 0...(some range)
│   │   ├── Thread in team: Processes its assigned j values
│   │   │   ├── For each j: SIMD vectorizes i loop with length ≤ 16
│   │   │   └── Vector operations on 16 i-values at once (if possible)
│   ├── Team 1: Processes next range of j values
│   └── ...
