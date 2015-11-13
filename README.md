## Gitolite Log File Parser with statistics
**Multiplatform Hierarchical Gitolite Log file parser to show SSH request statistics. Nested sorting is supported with various options. Single file solution.**
### options
* a - IP address
* c - command
* d - month day
* h - hour
* r - repository name
* u - user name
* w - week day

### compile on Windows with Visual Studio Compiler
*(tested with VisualStudio 2012, run from Developer Command Prompt for VS2012)*
  ```
  cl /EHsc /O2 /MD glp.cpp
  ```
### compile on Linux with gcc
  ```
  g++ -o glp glp.cpp
  ```
### usage

**linear parse gitolite log file**  
  *order is the same as original log file, each line is one transaction prefixed with index*
  ```
  glp gitolite-2015-07.log
  ```
**hierarchical parse gitolite log file**  
  *order of hierarcy depends on options order, leftmost column displays frequency counts*

* **Example 1** sort by user, then by repo  
  ```
  glp -ur gitolite-2015-07.log
  ```
* **Example 2** sort by month day, then by user  
  ```
  glp -du gitolite-2015-07.log
  ```
* **Example 3** sort by IP address, then by week day, finally by command  
  ```
  glp -awc gitolite-2015-07.log
  ```

**get option details**
  ```
  glp
  ```
**get version info**
  ```
  glp -v
  ```

### thanks!
