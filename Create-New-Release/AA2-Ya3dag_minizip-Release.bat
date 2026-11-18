@echo off
rem ************************************************************
rem  File: AA2-Ya3dag_minizip-Release  Author: R. Reinhard
rem
rem   Greate Ya3dag_minizip release ZIP file.
rem
rem   18.11.2026  First edition
rem               Create a zip file for a new release.
rem
rem ************************************************************

call AA1-Ya3dag_minizip-Data.bat

set ZipOut=Downloads\%projectName%_%releaseVersion%.zip
set MiniZip=Create-New-Release\Ya3dag_minizip -a -s -9 -i pspbrwse.jbf

rem --- Up one directory. Files are added relative to here. ...

cd .. 

echo --- delete existing target zip archive ...

rem ensure 'Downloads' is created

if not exist Downloads mkdir Downloads

rem existing target file

if exist %ZipOut% del %ZipOut%

echo --- add files to zip archive

%MiniZip% %ZipOut% LICENSE.txt
%MiniZip% %ZipOut% README.md
%MiniZip% %ZipOut% README-ReleaseHistory.txt
%MiniZip% %ZipOut% src/*
%MiniZip% %ZipOut% Create-New-Release/*

echo --- Eclipse IDE files

%MiniZip% %ZipOut% .cproject
%MiniZip% %ZipOut% .project
%MiniZip% %ZipOut% .settings/*

echo .
echo --- Done -----------------------------------
echo New archive: %ZipOut%
echo --------------------------------------------
echo .

echo on

@echo off

pause

:end
echo on
