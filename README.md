# Gitlet

## Overview
Gitlet is a version-control system that mimics some of the basic features of the popular system Git. (A version-control system is essentially a backup system for related collections of files.)

 The main functionality that Gitlet supports is:
- Saving the contents of entire directories of files.
- Restoring a version of one or more files or entire commits.
- Viewing the history of your backups. 
- Maintaining related sequences of commits.
- Merging changes made in one branch into another.(to be implemented...)

Note that we've simplified from Git by 
- Not dealing with subdirectories (so there will be one "flat" directory of plain files for each repository).
- Limiting ourselves to merges that reference two parents (in real Git, there can be any number of parents).

## Future features
- [x] status
- [x] checkout
- [ ] reset
- [ ] merge
- [ ] go remote...

## Acknowledgments
Thanks to UC Berkeley for creating this excellent project.

More details can be found in the documentation of this project at [CS 61B Project 3](https://inst.eecs.berkeley.edu/~cs61b/fa19/materials/proj/proj3/index.html)
