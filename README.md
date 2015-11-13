## Gitolite Log File Parser with statistics and performance analysis
**Multiplatform Hierarchical Gitolite Log file parser to show SSH request statistics. Nested sorting is supported with various options. This tool allows to diagnose various performance problems, like packet duration, access frequency etc. Single file solution easy to build on Windows, Linux, Mac etc.**
### options
* a - IP address
* d - month day
* e - delta time (duration)
* h - hour
* m - month
* n - minute
* r - repository name
* u - user name
* w - week day
* x - command
* y - year

### usage

**linear parse gitolite log file**  
  *order is the same as original log file, each line is one transaction prefixed with index*
  ```
  glp gitolite-2015-07.log
  ```
**hierarchical parse gitolite log file**  
  *order of hierarcy depends on options order, leftmost column displays frequency counts*

* **example 1** sort by user, then by repo  
  ```
  glp -ur gitolite-2015-07.log
  ```
* **example 2** sort by month day, then by user  
  ```
  glp -du gitolite-2015-07.log
  ```
* **example 3** sort by IP address, then by week day, finally by command  
  ```
  glp -awc gitolite-2015-07.log
  ```
* **example 4** sort by packet duration, then by user  
  ```
  glp -eu gitolite-2015-07.log
  ```

**get option details**
  ```
  glp
  ```
**get version info**
  ```
  glp -v
  ```
**get packet count**
  ```
  glp -c gitolite-2015-07.log
  ```

### compile on Windows with Visual Studio Compiler
*(tested with VisualStudio 2012, run from Developer Command Prompt for VS2012)*
  ```
  cl /EHsc /O2 /MD glp.cpp
  ```
### compile on Linux with gcc
*(tested with QNAP ARM and Intel NAS Gitolite server)*
  ```
  g++ -o glp glp.cpp
  ```
### thanks!
