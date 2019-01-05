copy build\i386\checked\ext2r.sys c:\winnt\system32\drivers\ > nul
echo Translating symbols...
"e:\DriverStudio\SoftIce\loader32.exe" /noprompt /translate /load /package c:\winnt\system32\drivers\netspeed.sys
e:\DriverStudio\SoftIce\icepack.exe > nul 

