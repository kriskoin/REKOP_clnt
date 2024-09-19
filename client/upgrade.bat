@echo off
if not exist temp goto fin
:existe
cd temp
upgrader.exe
cd ..
rd /S /Q temp
goto fin

:fin
