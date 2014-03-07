AMSimulation package with track prefiltering.

The track prefiltering is done by Filter.C.

Prefiltering the tracks to be used in the bankGeneration step of AMSimulation allows to reduce processing time by orders of magnitude.

The previous version of AMSimulation was running on all tracks from all sectors and the track selection was performed in the AMSimulation itself. However, the input files are too big to be stored in memory and the biggest bottleneck was the disk reading speed.
Prefiltering the tracks to those that are needed for bank generation in a given sector allows to achieve files sizes of ~ 2 GB, which are small enough to be stored in memory thus all but eliminating the speed bottleneck.

The time required for generating a bank in sector 0 with coverage 0.4 is reduced from ~ 4 hours to ~ 10 minutes.

The slow part is now moved to the track filtering step. However, while it is not trivial to parallelize the bankGeneration step (because it is necessary to check the coverage as the patterns are insterted in the bank), it is instead very simple to parallelize the track filtering step. This is done by running a job per file on the batch queues. Scripts for FNAL condor are available in the FilteringScripts folder.

