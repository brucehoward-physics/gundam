# GUNDAM — 風をあつめて

![](./resources/images/banner.jpg)

GUNDAM, for *Generic fitter for Upgraded Near Detector Analysis Methods*, is a suite
of applications which aims at performing various statistical analysis with different
purposes and setups. It has been developed as a fork of 
[xsllhFitter](https://gitlab.com/cuddandr/xsLLhFitter), in the context of the Upgrade
of ND280 for the T2K neutrino experiment.


The main framework offers a code structure which is capable of  handling parameters/errors
propagation on a model and compare to experimental data. As an example, GUNDAM includes
a likelihood-based fitter which was initially designed  to reproduce T2K's BANFF fit as
a proof of concept.

The applications are intended to be fully configurable with a set of YAML/JSON files, as
the philosophy of this project is to avoid users having to put their hands into the code
for each study. A lot of time and efforts are usually invested by various working groups
to debug and optimize pieces of codes which does generic tasks. As GUNDAM is designed for
maximize flexibility to accommodate various physics works, it allows to share optimizations
and debugging for every project at once.

## Showcase

![](./resources/images/samplesExample.png)

<details>
  <summary><b>Spoiler: More Screenshots</b></summary>

![](./resources/images/postFitCorrExample.png)

</details>



## How do I get setup?

### Prerequisites

There are several requirements for building the fitter:
- GCC 4.8.5+ or Clang 3.3+ (a C++11 enabled compiler)
- CMake 3.5+
- [ROOT 6](https://github.com/root-project/root)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)

### Shell setup

In this guide, it is assumed you have already defined the following bash environment
variables:

- `$REPO_DIR`: the path to the folder where your git projects are stored. This guide
will download this repo into the subdirectory `$REPO_DIR/gundam`.

- `$BUILD_DIR`: the path where the binaries are built. As for the previous variables,
this guide will work under `$BUILD_DIR/gundam`.

- `$INSTALL_DIR`: the path where the binaries are installed and used by the shell.
Same here: this guide will work under `$INSTALL_DIR/gundam`.

As an example, here is how I personally define those variables. This script is executed
in the `$HOME/.bash_profile` on macOS or `$HOME/.bashrc` on Linux, as they can be used
for other projects as well.

```bash
export INSTALL_DIR="$HOME/Documents/Work/Install/"
export BUILD_DIR="$HOME/Documents/Work/Build/"
export REPO_DIR="$HOME/Documents/Work/Repositories/"
```

If it's the first time you define those, don't forget to `mkdir`!

```bash
mkdir -p $INSTALL_DIR
mkdir -p $BUILD_DIR
mkdir -p $REPO_DIR
```

### Cloning repository

```bash
cd $REPO_DIR
git clone https://github.com/nadrino/gundam.git
cd $REPO_DIR/gundam
```

As a user, it is recommended for you to check out the latest tagged version of this
repository:

```bash
git checkout $(git describe --tags `git rev-list --tags --max-count=1`)
```

GUNDAM depends on additional libraries which are included as submodules of this git
project. It is necessary to download those:

```bash
git submodule update --init --recursive
```

### Updating your repository

Pull the latest version on github with the following commands:

```bash
cd $REPO_DIR/gundam
git pull
git submodule update --remote
git checkout $(git describe --tags `git rev-list --tags --max-count=1`)
cd -
```

### Compiling the code

Let's create the Build and Install folder:

```bash
mkdir -p $BUILD_DIR/gundam
mkdir -p $INSTALL_DIR/gundam
```

Now let's generate binaries:

```bash
cd $BUILD_DIR/gundam
cmake \
  -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_DIR/gundam \
  -D CMAKE_BUILD_TYPE=Release \
  $REPO_DIR/gundam/.
make -j 4 install
```

If you did get there without error, congratulations! Now GUNDAM is installed on you machine :-D.

To access the executables from anywhere, you have to update you `$PATH` and `$LD_LIBRARY_PATH`
variables:

```bash
export PATH="$INSTALL_DIR/gundam/bin:$PATH"
export LD_LIBRARY_PATH="$INSTALL_DIR/gundam/lib:$LD_LIBRARY_PATH"
```


### Gathering inputs

## I want to contribute!

## Lineage

GUNDAM was born as a fork of the *xsllhFitter* project which was developped and used by
the cross-section working group of T2K. The original project can be found on *gitlab*:
[https://gitlab.com/cuddandr/xsLLhFitter](https://gitlab.com/cuddandr/xsLLhFitter).

GUNDAM has originally been developed as an new fitter to perform T2K oscillation
analysis, and provide an expandable base on which future studies with the *Upgraded
ND280 Detectors* will be performed.


# OLD DESCRIPTIONS:

## Introduction

The goal of the Super-xsllhFitter is to provide a general purpose likelihood-based fit framework
for performing sentivity studies for the upgraded ND280.

The code is under very active development and to give some kind of stability, it is recommended
you checkout/download a tagged version of the fitter.

This document is currently all about the code for the fitter. For anything related to the principles
behind the fitter, browse any of the following technotes: TN214, TN261, TN263, TN287, TN337, TN338.

## Installation

There are several requirements for building the fitter:
- GCC 4.8.5+ or Clang 3.3+ (a C++11 enabled compiler)
- CMake 3.5+
- ROOT 5 or 6

ROOT needs either Minuit or Minuit2 and optionally the MathMore package enabled to perform the minimization. The recommendation is to have both Minuit and Minuit2 enabled. In addition it is highly recommended to have a working OpenMP installation to take advantage of parallelism when running the code.

To checkout a tagged version of the code using git:

```bash
git clone https://gitlab.com/cuddandr/xsLLhFitter.git
git tag # find your suitable tag
git checkout -b remotes/origin/ND280UpFit <tag>
```

This setup guide assumes that you've already set the following env variables.

```bash
export REPO_DIR="/folder/path/where/your/store/your/git/repository"
export BUILD_DIR="/folder/path/where/your/store/your/build/files"
export INSTALL_DIR="/folder/path/where/your/store/your/installed/bin/files"
```

In this guide `$REPO_DIR/xsllhFitter` is where the source code is placed, in `$BUILD_DIR/xsllhFitter` there will be your Makefiles and finally `$INSTALL_DIR/xsllhFitter` will contain the folders `bin` `lib`.

Before building the code, you need to source the setup script.

```bash
source $REPO_DIR/xsllhFitter/setup.sh
```

The first time this script is run it will notify you that it cannot find the build setup script, this is normal. The fitter is designed to be built in a build directory specified by the user and is configured using CMake.

To build (with default settings):

```bash
cd $BUILD_DIR/xsLLhFitter
cmake \
      -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_DIR/xsLLhFitter \
      -D CMAKE_BUILD_TYPE=RELEASE \
      $REPO_DIR/xsLLhFitter/.
```

The default build is `DEBUG`, which compiles the libraries statically and includes debugging symbols. The other build type is `RELEASE`, which can be enabled by either calling cmake with `-DCMAKE_BUILD_TYPE=RELEASE` or by using the ccmake command. The `RELEASE` build enables compiler optimizations, disables debug symbols, and builds/links the libraries as shared objects. Other options can be passed to CMake by using `-DOPTION_NAME=VALUE` when invoking cmake, or by using ccmake.

There are a few extra options to configure the build. The default option is listed in brackets:
- CMAKE_CXX_EXTENSIONS [OFF]: Enable GNU extensions to C++ language (-std=gnu++11)
- CXX_MARCH_FLAG [OFF]: Enable cpu architecture specific optimizations
- CXX_WARNINGS [ON]: Enable most C++ warning flags
- COLOR_OUTPUT [ON]: Enable colored terminal output

### CMake Finding Incorrect Compiler

CMake may not find the correct compiler when there are multiple available. If this is the case, set the compiler manually by either passing the `-DCMAKE_CXX_COMPILER` and `-DCMAKE_C_COMPILER` variables when configuring or exporting the `CC` and `CXX` variables before configuring. All previous build files will need to be deleted for cmake to reconfigure properly. For example:

```bash
$ cmake -DCMAKE_CXX_COMPILER=/path/to/compiler/executable -DCMAKE_C_COMPILER=/path/to/compiler/executable ../
```

Or by exporting shell variables:

```bash
$ export CC=/path/to/compiler/executable
$ export CXX=/path/to/compiler/executable
$ cmake ..
```

For future use, the root setup.sh script will perform all the necessary setup to run the fitter. Once configured with CMake, only the `make install` step needs to be performed if the code needs to be rebuilt/recompiled.

## Running the Code

The Super-xsllh Fitter is built to be a framework of tools which are designed to work together. There are the tools that produce inputs in the correct format, the main fit program, the error propagation, and a number of scripts which may or may not be useful. In addition, there is a set of programs designed to run T2KReWeight and produce splines which is currently not included in this repository.

Good luck.

## xsllhFit

This program performs the minimization and fit to the data/MC producing a set of best-fit parameters and best-fit covariance matrix.

It is designed to be run with a JSON configuration file. The usage of the command is as follows:
```bash
$ xsllhFit [-n] [-h] -j </path/to/config.json>
```
The `-j` flag is required and is config file for the fit. The `-n` flag does a "dry-run" of the fit where it is initialized, but the minimization is not performed. The `-h` displays the help output and available options.

### Configuration file

Examples in the repository. Full description coming soon.

## xsllhCalcXsec

This program performs the error propgation and cross-section calculation using the post-fit file from `xsllhFit`.

It is designed to be run with a JSON configuration file, and has CLI options which override certain options in the configure file. The usage of the command is as follows:
```bash
$ xsllhCalcXsec [-i,o,n,p,m,t,h] -j </path/to/config.json>
USAGE: xsllhCalcXsec
OPTIONS:
-j : JSON input
-i : Input file (overrides JSON config)
-o : Output file (overrides JSON config)
-n : Number of toys (overrides JSON config)
-p : Use prefit covariance for error bands
-m : Use mean of toys for covariance calculation
-t : Save toys in output file
-h : Print this usage guide
```
The `-j` flag is required and is the config file for the error propagation. The other flags are optional and take priority over the same options in the configure file if applicable.

### Configuration file

Examples in the repository. Full description coming soon.

## xsllhTreeConvert

This program provides a base for translating event trees into the format expected by the fit code. This has basic support for HighLAND2 files, but still may require analysis specific tweaks. The "flattree" for the fit contains a small set of variables for both selected and true events.

It is designed to be run with a JSON configuration file. The usage of the command is as follows:
```bash
$ xsllhTreeConvert -j </path/to/config.json>
```
The `-j` flag is required and is the config file for the fit. Currently there are no other options; all settings are specified in the configure file.

### Configuration file

Examples in the repository. Full description coming soon.

## xsllhDetVariations

This program performs the calculation of the detector covariance for HighLAND2 files. The `all_syst` tree must be enabled in HighLAND2 along with the systematic variations to be included in the covariance matrix.

It is designed to be run with a JSON configuration file. The usage of the command is as follows:
```bash
$ xsllhDetVar -j </path/to/config.json>
```
The `-j` flag is required and is the config file for the fit. Currently there are no other options; all settings are specified in the configure file.

### Configuration file

Examples in the repository. Full description coming soon.

## xsllhXsecCov

This program translates a text file containing the cross-section covariance matrix into a ROOT file for the fit.

It is designed to be run using a series of command line options. The usage of the command is as follows:
```bash
$ xsllhXsecCov
USAGE: xsllhXsecCov -i </path/to/cov.txt>
OPTIONS
-i : Input xsec file (.txt)
-o : Output ROOT filename
-m : Covariance matrix name
-b : Add value to diagonal
-r : Parameter mask
-C : Calculate correlation matrix
-I : Build INGRID covariance
-S : Store as TMatrixT
-h : Display this help message
```
The `-i` flag is required and is the cross-section covariance in a text file. The rest of the options are optional. The lowercase options take required parameters while the uppercase options are simple flags.
