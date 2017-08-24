The following instructions worked for me on Ubuntu 16.04. They may work for you on your machine; if they do not, let me know and I will try to assist you.

If you get any nice results using this software, please let me know! The software is based off algorithms described in:
*Computing Tropical Prevarieties in Parallel*. Anders Jensen, Jeff Sommars, and Jan Verschelde. Proceedings of the 8th International Workshop on Parallel Symbolic Computation (2017).

### Install Instructions: ###

* Begin by installing a branch of PPL
    ```
    git clone git://git.cs.unipr.it/ppl/ppl.git
    cd ppl
    git checkout devel
    autoconf (Don't be concerned about errors)
    autoreconf
    ./configure --enable-thread-safe
    make
    make install
    ```
* Install tcmalloc (http://goog-perftools.sourceforge.net/doc/tcmalloc.html)

* Install DynamicPrevariety
    ```
    git clone git@github.com:sommars/DynamicPrevariety.git
    cd DynamicPrevariety
    make
    ```
### Using DynamicPrevariety: ###

* DynamicPrevariety takes in support sets of Newton polytopes written to file; for examples, see the examples folder. To compute the prevariety of the cyclic-8 roots problem, run ./dynamicprevariety ./examples/cyclic/cyclic8. If you wish to use multiple threads, specify an integer after the file name.
