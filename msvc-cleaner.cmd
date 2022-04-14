@echo off

echo.
echo Removing files ...
echo.

del    *.VC.db  /s/q
del    *.bsc    /s/q
del    *.pdb    /s/q
del    *.iobj   /s/q
del    *.ipdb   /s/q
del    *.ilk    /s/q
del    *.ipch   /s/q
del    *.obj    /s/q
del    *.sbr    /s/q
del    *.tlog   /s/q
del    *.suo    /s/q

@REM msbuild
del    *.metaproj    /s/q
del    *.metaproj.tmp    /s/q

echo.
echo Removing build folders ...
echo.

for /d /r . %%d in (.vs .vscode .\build\* .\build) do (

     if exist "%%d" (
         rd /s/q "%%d" 
         if not exist "%%d" ( echo [%%d] Removed! )
         echo.
     )
)

echo Done!
pause