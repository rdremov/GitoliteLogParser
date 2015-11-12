GitoliteLogParser
=================
# Multiplatform Hierarchical Gitolite Log file parser to show SSH request statistics.
## Available options are:
* a - IP address
* c - command
* d - month day
* h - hour
* r - repository name
* u - user name
* w - week day

## To compile on Windows with Visual Studio Compiler:
(tested with VisualStudio 2012, run from Developer Command Prompt for VS2012)

	`cl /EHsc /O2 /MD glp.cpp`

## To compile with gcc:

	`g++ -o glp glp.cpp`

## After you build it usage is straightforward.

To get option details:
	`glp`

To get version:
	`glp -v`

To linear parse gitolite log file:
	`glp gitolite-2015-07.log`

To hierarchical parse gitolite log file. Order of hierarcy depends on options order

*Example 1. Sort by user, then by repo
	`glp -ur gitolite-2015-07.log`

*Example 2. Sort by month day, then by user
	`glp -du gitolite-2015-07.log`

*Example 3. Sort by IP address, then by week day, finally by command
	`glp -awc gitolite-2015-07.log`

Leftmost column displays statistics (counts).