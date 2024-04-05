# Scheduler/Dispatcher Simulation

This project implements a discrete event simulation (DES) of various scheduling algorithms as discussed in the Operating Systems course (CSCI-GA.2250-001 Spring 2023) under Professor Hubertus Franke. The simulation is designed to observe the effects of different scheduling policies on a set of processes/threads executing within a system.

## Overview

The simulation models a single-CPU system executing a series of processes. Each process undergoes a series of state transitions through its lifecycle, emulating the operation of an operating system's scheduler/dispatcher. The primary focus is to compare the performance and behavior of six different scheduling policies:

- First-Come, First-Served (FCFS)
- Last-Come, First-Served (LCFS)
- Shortest Remaining Time First (SRTF)
- Round Robin (RR)
- Priority Scheduling (PRIO)
- Preemptive Priority Scheduling (PREPRIO)

## Simulation Model

The system progresses through discrete time steps, where at each step, various events such as process creation, CPU bursts, and IO bursts occur. These events influence the state of the processes in the system, governed by the scheduling algorithm in use. The simulation is based on the principle of Discrete Event Simulation (DES), focusing on the chronological sequence of events marking changes in the system state.

## Implementation Details

The simulation has been implemented in C++, utilizing object-oriented principles to model the components of the system, including processes, events, and the scheduler. The scheduling algorithms are implemented as separate classes inheriting from a common scheduler interface, allowing for polymorphic behavior in managing processes according to each algorithm's rules.

### Scheduling Policies

- **FCFS**: Processes are served in the order of their arrival.
- **LCFS**: The last arriving process is served first.
- **SRTF**: Processes with the shortest remaining time are prioritized.
- **RR**: Processes are served in a round-robin fashion, with a configurable time quantum.
- **PRIO**: Processes are scheduled based on priority, with optional support for multiple priority levels.
- **PREPRIO**: An extension of PRIO with support for preempting running processes based on priority.

## Building and Running the Simulation

### Prerequisites

- A C++ compiler (e.g., g++, clang++)
- Makefile (for building the project)

### Compilation

To compile the simulation, navigate to the project directory and run:

```bash
make

### Running
./scheduler [-v] [-t] [-e] [-p] [-i] [-s<schedspec>] inputfile randfile
-v: Verbose mode.
-t, -e, -p, -i: Additional options for detailed tracing and debugging.
-s<schedspec>: Specifies the scheduling policy to use.
inputfile: Specifies the path to the input file containing process specifications.
randfile: Specifies the path to the file containing random numbers used for generating CPU and IO bursts.
