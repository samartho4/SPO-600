Introduction
In this lab, I built the latest development version of the GCC compiler from source on two different architectures:

x86-001 (x86_64)

aarch64-002 (ARM64)

The goal was to understand the build process of a large software project, analyze build times, and test incremental compilation.

Build Process
1️⃣ Cloning & Setting Up the Build
I cloned the GCC repository and configured the build:


Copy
shCopyEditgit clone https://github.com/samartho4/gcc.git
cd gcc
mkdir $HOME/gcc-build-001
cd $HOME/gcc-build-001
$HOME/gcc/configure --prefix=$HOME/gcc-test-001 --enable-languages=c,c++ --disable-multilib
2️⃣ Compiling & Installing GCC
The compilation was executed using:


Copy
shCopyEdittime make -j$(nproc) |& tee build.log
make install
After installation, I verified my build:


Copy
shCopyEdit$HOME/gcc-test-001/bin/gcc --version
Build Times Comparison
Server	Initial Build Time	Modified File Rebuild	Null Rebuild
x86-001	47 min 20 sec	48.656 sec	13.367 sec
aarch64-002	110 min 18 sec	3 min 11 sec	22.741 sec
Observations
The x86 build was significantly faster than the aarch64 build.

Rebuilding after modifying passes.cc was much faster than the initial build.

Null rebuilds (where nothing changes) were nearly instant because make skipped unnecessary recompilation.

Proving the Custom GCC Build
🖥️ x86-001

Copy
gcc --version && echo "-----------------" && $HOME/gcc-test-001/bin/gcc --version
📌 Output:


Copy
scssCopyEditgcc (GCC) 14.2.1 20240912 (Red Hat 14.2.1-3)
-----------------
gcc (GCC) 15.0.1 20250207 (experimental)
✔️ Confirmed: The system’s default GCC is 14.2.1, but my custom-built GCC 15.0.1 is installed and functional.

📱 aarch64-002

Copy
gcc --version && echo "-----------------" && $HOME/gcc-test-001/bin/gcc --version
📌 Output:


Copy
scssCopyEditgcc (GCC) 11.3.1 20220421 (Red Hat 11.3.1-3)
-----------------
gcc (GCC) 15.0.1 20250207 (experimental)
✔️ Confirmed: The system’s default GCC is 11.3.1, and my custom-built GCC 15.0.1 is installed and working.

Testing GCC with a Sample Program
To verify that my new GCC was fully functional, I compiled a simple C program.


Copy
shCopyEditecho '#include <stdio.h>\nint main() { printf("Hello, GCC!\\n"); return 0; }' > test.c
gcc test.c -o test
./test
📌 Output:


Copy
CopyEditHello, GCC!
✔️ Confirmed: My GCC build was successful and working correctly.

Conclusion
Building GCC was a time-intensive but insightful process that deepened my understanding of large-scale compilation.

The x86-001 build was much faster than aarch64-002, showing the impact of hardware differences.

Make’s incremental and null rebuilds saved significant time, highlighting the efficiency of modern build systems.

Overall, this lab provided valuable insights into compiler construction, optimization, and cross-platform software development.
