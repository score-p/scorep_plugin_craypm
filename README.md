# PM Plugin for Score-P

## Compilation and Installation

### Prerequisites

To compile this plugin, you need:

* GCC compiler

* CMake

* Score-P

### Build Options

* `-DCMAKE_INSTALL_PREFIX`

    The folder that holds the lib and include folder for Score-P (should be set automatically).

* `-DENABLE_FRESHNESS_COUNTER` (default=on)

    Enables measurement of freshness counter.

### Building

1. Create build directory

        mkdir build
        cd build

2. Invoking CMake

        cmake ..

3. Invoking make

        make

4. Installing

        make install

    or copy it to a location listed in `LD_LIBRARY_PATH` or add current path to `LD_LIBRARY_PATH` with

        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`

## Usage

1. Compile your program as usual with Score-P.

2. Set the following environment variables:

        export SCOREP_METRIC_PLUGINS="pm_plugin"
        export SCOREP_METRIC_PM_PLUGIN="all"

3. Run your program as usual.

4. After run, the metric should be included.

### If anything fails

1. Check whether the plugin library can be loaded from the `LD_LIBRARY_PATH`.

2. Check whether you are allowed to read `/dev/cpu/*/msr`.

3. Write a mail to the author.

## Authors

* Mario Bielert (mario.bielert at tu-dresden dot de)
