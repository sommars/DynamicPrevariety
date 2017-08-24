The following instructions worked for me on Ubuntu 16.04. They may work for you on your machine; if they do not, let me know and I will try to assist you.

If you get any nice results using this software, please let me know! The software is based off algorithms described in:
*Computing Tropical Prevarieties in Parallel*. Anders Jensen, Jeff Sommars, and Jan Verschelde. Proceedings of the 8th International Workshop on Parallel Symbolic Computation (2017).

### Install Instructions: ###

* Begin by installing a branch of PPL
    1. git clone git://git.cs.unipr.it/ppl/ppl.git
    2. cd ppl
    3. git checkout devel
    4. autoconf (Don't be concerned about errors)
    5. autoreconf
    6. ./configure --enable-thread-safe
    7. make
    8. make install

* Install tcmalloc (http://goog-perftools.sourceforge.net/doc/tcmalloc.html)

* Install DynamicPrevariety
    1. git clone git@github.com:sommars/DynamicPrevariety.git
    2. cd DynamicPrevariety
    3. make

### Using DynamicPrevariety: ###

* DynamicPrevariety takes in support sets of Newton polytopes written to file; for examples, see the examples folder. To compute the prevariety of the cyclic-8 roots problem, run ./dynamicprevariety ./examples/cyclic/cyclic8. If you wish to use multiple threads, specify an integer after the file name.
