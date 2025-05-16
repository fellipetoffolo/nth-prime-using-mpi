# Case study analysis: Computer cluster with 16 RK3228A TV Boxes

This example aims to show how this program can be used to measure performance gains
by increasing the number of CPUs (process) running the tasks to find the nth prime number

## Hardware used

For this example, we used 16 units of the UniTV S1 TV Box. It has the following specifications:

Component   | Specification
------------|-------------------
RK3228A CPU |       1,2 Ghz
RAM         |       1 GB
Storage     |       8 GB
Network     | Realtek rtl8723ds

And bellow there are some illustrative images:

|Circuit      | Box        |
|-------------|------------|
| <img src=".assets/unitvs1-placa.png" alt="TV Box circuit" width="200"> | <img src=".assets/unitvs1-box.png" alt="TV Box" width="220">|

Moreover, a Cisco switch Catalyst 2960 was also used to distribute the data between the units and
make the data trasmission suitable, as it would be very slow by any other mean. Obviously, network
cables and power supply were also needed, but they are not gonna be shown here, as they are very
common pieces of hardware. The following figure illustrates the switch mentioned:

![Cisco Catalyst switch](.assets/CiscoCatalyst.png)

## Script for running the MPI program with increasingly more processes

A simple bash script was written to run the program with the same command-line argument, but with increasingly
more processes (units), so that every execution time could be compared. The script is the folowing:

```bash
#!/bin/bash

PROBLEM_SIZE=10000000

for NP in 4 8 12 16 20 24 28 32 36 40 44 48 52 56 60 64; do
    echo "Running with $NP process"
    mpirun -np $NP --hostfile cluster ./nth_prime $PROBLEM_SIZE
```

## Results and performance gain

The results are shown on the figure bellow, where the execution time on the y axis is related to the number of processing units on the x axis.

![Execution time](.assets/nth_prime_graph.png)

## Comparison to Modern Desktops

![Execution time](.assets/comparison_nth_prime.png)