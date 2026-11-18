README-ReleaseHistory.txt
-------------------------

Release history of Ya3dag_minizip.

------------------------------------------------------
 V1.00  18.01.2026

          First release of this package on GitHub.
------------------------------------------------------

18.01.2026 RR: * Reworked project to work with the Eclipe IDE.
                 Before the CodeBlocks IDE was used.
                 The application generated is now a 64-bit application.
               * Only the source files that are needed have
                 been transferred to the new project.
               * Minor adjustments to a few files so that they can be
                 compiled in the new environment.

29.08.2026 RR: * Reworked AddFileToZipRecursive() skip of current/parent directory.
                 Now, explicit test for current or parent directory.
                 Before the file/directory was skipped if the first character was a dot.
                 This allows files or directories beginning with a dot.
               * For none silent mode added printout of compressed files per sub directory.

19.01.2014 RR: * Compile Ya3dag_minizip without need for the zlib1.dll file.
                 Just compile all needed file (inside subdirectory zlib) in
                 place of linking a library.
               * Using the MinGW compiler, the program Ya3dag_ninizip makes
                 wildcard processing itself on the commandline preprocessing.
                 To disable this unwanted feature:
                 In minzip.c add line
                    int _CRT_glob = 0;    // disable wildcard globbing

29.07.2012 RR: * Always do the recursive directory walking.
                 No limits of directorys by given file mask.
               * Option -n to explicit disable the recursive
                 directory walking.

11.01.2010 RR: * Converted Workspace to Visual C++ 2008
                 and compiled.

09.09.2008 RR: * Moved project into Ya3adag_src directory and
                 reworked workspace settings.
               * added Ya3dag icon to executable.
               * renamed executable to Ya3dag_minizip.exe

04.07.2008 RR: * modified to allow wildcards in the files to add

04.01.2008 RR: * First edition

