The following instructions worked for me on Ubuntu 16.04. They may work for you on your machine; if they do not, let me know and I will try to assist you.

If you get any nice results using this software, please let me know!

The software is based off algorithms described in:
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
* (Optional) Install tcmalloc (http://goog-perftools.sourceforge.net/doc/tcmalloc.html) This will lead to substantial improvements in parallel performance.

* Install DynamicPrevariety
    ```
    git clone git@github.com:sommars/DynamicPrevariety.git
    cd DynamicPrevariety
    make PPLPATH=/home/jeff/Software/ppl/ ALLOC=libtcmalloc_minimal.so.4.2.6
    ```
    Note that the PPLPATH and ALLOC name will likely be different for you. If you choose not to install tcmalloc, exclude that argument from the make command. If you installed ppl as root, it may not be necessary for you to pass in the absolute path to ppl.

### Using DynamicPrevariety: ###

* DynamicPrevariety takes in support sets of Newton polytopes written to file; for examples, see the examples folder. To compute the prevariety of the cyclic-8 roots problem, run ./prevariety ./examples/cyclic/cyclic8.
* There are several options that can be set. Run `./dynamicprevariety -help` for more information.
