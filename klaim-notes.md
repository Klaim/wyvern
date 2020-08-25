Wyvern: CMake To Build2 Tool
----------------------------

Goals
-----

A. Extract target information from an installed CMake project package.
B. Extract target information from a source/build-directory CMake project.
C. Expose targets to build2 so that it can use the information of these potential dependencies.

Do we really need to handle the case with the sources? Maybe just case A is necessary?

Current Plan
------------

(see reports below - data retrieval would be based on CMake's file-api (which is limited))
> I plan to try something like:
> 1. invoke cmake with a target with no dependency, get the flags/args being used
> 2. invoke cmake with the same target where we add the dependency to the package target we want, get the flags/args being used
> 3. extract the difference between both captured data (so that I know which part have been added when we added the dependency)
> 4. expose the data to output in a way useful to build2
>
> If I manage to get point 3 right, it should work with evolution of CMake.

Point 4 is not necessary, just send the code to Boris.


TODO
----

 - fix missing information about inter-cmake-package dependencies;
 - fix name of dependencies targets not being preserved in the output;
 - allow libwyvern tests to run from any directory, not just the root of this repository;
 - add a cli program to run the `extract_dependencies` function on any CMake project installed;
   - handle all options we need
 - try to run the cli on a few important projects:
   - fmt (because it's simple and easy)
   - boost
   - gtest? (because it's non-trivial)
   - Ogre? (because it's bullshit-hard)
 - try to run the cli with libraries installed via `vcpkg`
 - if possible, make clear in the dependencies info if library targets are static or shared (not sure it's information kept by cmake...);

### DONE

 - output the info found (at least on stdout)
 - start write the tool to read the basic info:
   - list of targets, with for each target:
     - programming language
     - include dirs
     - binaries paths (or just the dirs where to find them?)
     - link flags that users should use
     - preprocessor flags that user should use
     - compilation flags that user should use

 - gather dependencies for the tool:
   - boost(process) - used butl isntead
   - something for handling arguments and help
   - json library (modern json?)
   - format library (fmt?)

 - play with cmake file-api in the case of a cmake-package
 - see how it looks when you have a cmake package referencing other packages (through find_package)
 - find how to distinguish include dirs that should be added to a library users, vs the other libraries (private)


Notes
-----

1. cmake exposes information about targets depending on the build mode.
Therefore, we need to specify which mode we want the target in.

2. Steps the tool needs to take
  0. detect the kind of target directory: sources (CMakeLists.txt is here), build (CMakeCache.txt), install
  1. invoke cmake:
    - if we target a source directory: invoke cmake on it with a custom build dir (optionally let the user specify the build dir)
    - if we target a build directory: invoke cmake on it (we'll use that build directory)
    - if it's an install directory: create a CMakeLists.txt with just find_package invoking the install dir.
  2. request info through the cmake file api
  3. read wanted infos in the mode we want:
    - targets: names, artefact paths, compilation flags (for users at least), dependencies, (programming language?), shared or static or exe?, etc.
  4. generate equivalent buildfile to generate stubs packages?
  5.


3. program's arguments should be:
 - mode (release by default)
 - target source, build or install directory
 -

4. BEWARE of taking into account the dependencies of the found targets.
    We have to keep track of the dependencies being local OR the remote ones.


Reports
-------

## 2020-01-17 (slack):

> boris  2:31 PM
> @klaim Thanks for looking into this. Just to confirm, the approach suggested by @rob.maynard where we create a "consumer" project and use the file api on that project in order to get the information about the "target" project (e.g., a library) is also a no-go?
>
> klaim  2:41 PM
> @boris I need to do some more experiment to state if it's a total no-go. I suspect some useful information can be extracted, probably not all we need.
>
> klaim  2:51 PM
> As a concrete example, @rob.maynard’s suggestion when trying to get info about fmt:
> find_package(fmt REQUIRED)
> add_executable(test test.cpp)
> target_link_libraries(test fmt::fmt)
> In the file-api output we get the names of the targets you see in this file, test, but not fmt::fmt . Associated with test data, we can see the include directories, compilation flags and link arguments being correctly generated with the necessary flags/info/data coming from fmt::fmt but it is not stated that it comes from a target fmt::fmt
> This means that in simple cases, if we just try to link to a simple library, we can theorically extract the additional flags, include directories and link arguments we find in the output of the file-api. However as soon as that library have itself other dependencies, we cannot know which flags, include directories, etc. comes from which dependency targets.
> It's all reduced to just compilation and link command line arguments for the leaf consuming targets.
> 2:54
> To me it looks like that api is not designed to let you know how the dependencies relate, it just tell you how to compile the project and did the deduction of the command lines for you, so you lose the information. (clarification: it does tell you the relationships of the targets in the same cmake project sources, but not when the targets comes from external packages) (edited)
> 2:56
> I guess that qualifies as a no-go.
>
> boris  3:43 PM
> Hm, while I agree it would be nice to have the more precise information about which options correspond to which dependencies, we have the same situation in the pkg-config case. That is, we get compile and link options  that are required to compile and link the library and just have to live with that. So provided we can be reasonably certain which options come from the library rather than from the project itself, this could work, I think.
>
> klaim  3:48 PM
> I think you're right, it's almost similar in that view.
> 3:49
> So the info would be less precise than what cmake knows but enough to use the library. Ok I'll try to continue in that direction then.
>
> rob.maynard  4:05 PM
> The back propagation of which flags come which targets is as you pointed out is lost. But for more complex examples this relationship is less useful. You will have a compile line with  a single -std=c++17 flag but that would map to 10+ targets before we de-duplicate it
>
> boris  4:19 PM
> @rob.maynard Is there any way to distinguish which options belong to the project/configuration itself and which come from the library? (edited)
>
> rob.maynard  5:44 PM
> I don’t think so. But I haven’t spent significant time using the api. I think in general we wouldn’t as we just care about providing the final set of flags
> Added to your saved items
>
> klaim  5:51 PM
> I plan to try something like:
> 1. invoke cmake with a target with no dependency, get the flags/args being used
> 2. invoke cmake with the same target where we add the dependency to the package target we want, get the flags/args being used
> 3. extract the difference between both captured data (so that I know which part have been added when we added the dependency)
> 4. expose the data to output in a way useful to build2
> If I manage to get point 3 right, it should work with evolution of CMake.
>
> boris  6:20 PM
> That's a pretty clever idea. I was thinking of adding some bogus option and hoping that CMake just appends library options after it. But this is definitely better/cleaner.
>
> klaim  6:21 PM
> I think this way it would also work whatever the options you send to cmake.
> Though, in practice, point 3 might not be feasable correctly hahaha
>
> boris  6:22 PM
> I think it should be good enough for most practical cases.
>
> klaim  6:22 PM
> For point 4, what format do you think would be the most useful? Something like config.cxx.poptions=... would work but might be overkill?
> 6:23
> I mean, if you pass that output to b then all your targets will use these options. (edited)
>
> boris  6:26 PM
> Yes, if I remember correctly the idea was to do it as a build system module. Specifically, we will generalize the pkg-config logic to allow external modules to supply this information and then it will be a matter of just hooking into this API and providing compile/link options. So if you can provide two vector<string> , that should be sufficient.
> 6:29
> If you could implement a prototype (program) in C++ (that maybe uses libbutl for file/path/process support) that implements step 1-3 and produces these two arrays, then it should be pretty straightforward to convert it to a build system module.
>
> klaim  6:57 PM
> Ok so I'll skip step 4 and notify you when I'm satisfied with what I have.


## 2020-01-16 (slack):

Klaim:

> Quick status report about the "cmake->build2" project: it's not trivial at all
> cmake file api does not provide the information we want: it provides all the data necessary for a tool to know how the source code of the projects will be compiled BUT It is impossible to distinguish public information from private one. For example we can get the compilation flags that will be used to compile the project's sources, not the flags that the library users should use (same for headers etc.) I tried in several ways to acquire that information through this API then discussed in the cmake channel here and confirmed it was not possible. So that file API does not do what I need. It's mainly useful to know which targets exist in a cmake project (not a cmake package though)
> Looking at how Meson does it, basically they have their own cmake parser and completes it with the cmake file api and some other info. That does not seem sustainable.
> As it was put in the cmake channel here: "it would be nice if there was libcmake" but it's not for today or tomorrow...
> It looks like the best strategy to extract info of targets at the moment is to write a cmake file that does the extraction itself by generating some output at configuration time. That would use target names extracted from the cmake-file-api to know where to look at. I didn't explore this yet, so I'll report about it later.
> I also have some additional notes somewhere but at this stage I don't think it's important yet.


