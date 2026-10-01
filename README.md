# CS1D Assignment 5

This program simulates an afternoon in an emergency room with two priority
queues. `main.cpp` contains both queue implementations required by the
assignment:

1. `AuthorPriorityQueue` is a vector-backed binary heap.
2. `std::priority_queue` is the C++ Standard Library implementation.

Life-threatening patients have priority over everyone else. Otherwise, the
patient who waited the longest is treated first. Every appointment requires 25
minutes. A life-threatening arrival interrupts the current appointment, and
the patient returns to the queue with the treatment time still remaining.

The table leaves Cathy Coughing's waiting time blank, so the default data uses
zero hours and records that assumption here. Sam Sneezing arrives at 1:11 PM
and Paula Pain arrives at 2:16 PM. Times are printed in 24-hour format.

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o assignment5
./assignment5
```

The output lists every treatment segment, including interruptions, for both
priority queues and reports whether the schedules match. The simulation takes
a vector of patients, so its logic is not tied to the pictured names or data.
