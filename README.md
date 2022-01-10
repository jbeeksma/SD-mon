# SD-mon

SD-Mon project

Driel, 10 January 2022

This project was started to develop a SD card file system for a 6309 single board computer (developed by Tom LeMense).

The filesystem is developed with the following thoughts in mind:
- The SD card is much faster than a floppy disk, so I don't mind many read-write operations
- The SD card has ample space for a vintage 8-bit computer system
- I have developed my own file allocation and directory structures
- SD cards can be split into max 1 partitions
- A drive letter can be assigned to a partition
- Each partition can have a separate boot file

At this stage I'd like to call 0.1 I can format an SD card with an initial partition and a root dir in the partition.

Later come the files, and the OS commands to use it.
