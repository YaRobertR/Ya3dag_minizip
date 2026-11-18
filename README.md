# Ya3dag_minizip

Yet another command-line program for compressing files into a ZIP archive.

#### What is the purpose of the project?

The command line application Ya3dag_minizip is used for creating zip files.
This is done by recursive walking in directory trees and adding the files.
The files_to_add allows wildcards in the name. As a filter, the ignore
argument can suppress some of the files traversed.

#### Usage

This application is used to create download files for releases. To do this, use the application in a windows batch file.

**Usage :** Ya3dag_minizip [-o] [-a] [-s] [-n] [-0 to -9] [-i string] file.zip [files_to_add]
    -o  Overwrite existing file.zip
    -a  Append to existing file.zip
    -s  silent (less text output)
    -n  no recursive directory walking
    -i  ignore if string is part of filename to add
    -0  Store only
    -1  Compress faster
    -9  Compress better

#### Installation

- Windows is the only supported operating system.

- Download the latest release from my [releases page](https://github.com/YaRobertR/Ya3dag_minizip/releases)

- See the 'Example' directory for an executable and examble files

#### Examples

* Example from CD_Release_Ya3dag.bat:
  
      ..\Ya3dag_minizip -9 -a -s -i pspbrwse.jbf %tempDir%\BaseQ2\Q2T_BaseQ2.pkz textures\*.*
  
  Add all files under the textures subdirectory to the zip file named Q2T_BaseQ2.pkz.
  Paintshop browser files (pspbrwse.jbf) are not added.

* Example from CD_Release_Ya3dag_Source.bat:
  
      %Q2Dir%\Ya3dag_minizip -9 -a -s -i \release\ -i \debug\ -i .ncb -i .opt -i .plg %tempDir%\Ya3dag_V%releaseVersion%_Source_Quake2Terain.zip Quake2Terain\*.*
  
  Collect all source files for Quake2Terain to XXX_Quake2Terain.zip. All intermedia files
  (.obj, .ncb, ...) are ignored.

* There is also a sample batch file in the “Create-New-Release” subdirectory.
  This batch file is used to create a download package for a new release.

#### Credits

The application Ya3dag_minizip is based on

  MiniZip 1.01b, demo of zLib + Zip package written by Gilles Vollant
  more info [here](http://www.winimage.com/zLibDll/minizip.html).
