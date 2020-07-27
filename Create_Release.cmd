for %%i in (1,2,3,4,5,6) do ( 
 C:\TI\ccsv8\eclipse\eclipsec -noSplash -data "C:\Users\Max\workspace_v8" -application com.ti.ccstudio.apps.projectBuild -ccs.projects Syntherrupter_Tiva -ccs.configuration Release_%%i
 robocopy Syntherrupter_Tiva\Release_%%i Syntherrupter_Firmwares Syntherrupter_Tiva.bin
 ren Syntherrupter_Firmwares\Syntherrupter_Tiva.bin Syntherrupter_Tiva_%%i_Coils.bin
 )
ren Syntherrupter_Firmwares\Syntherrupter_Tiva_1_Coils.bin Syntherrupter_Tiva_1_Coil.bin
robocopy Syntherrupter_Nextion Syntherrupter_Firmwares Syntherrupter_Nextion.HMI
cd Syntherrupter_Firmwares
"C:\Program Files\WinRAR\winrar" a -afzip -m5 ..\Syntherrupter_Firmwares.zip
cd ..
del /q Syntherrupter_Firmwares
md Syntherrupter_Firmwares