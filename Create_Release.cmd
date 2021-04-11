rem del /q Syntherrupter_Firmwares
md Syntherrupter_Firmwares
rem Start Nextion Build VM.
rem "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" controlvm "Windows 7 Nextion VBox" poweroff
rem "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" snapshot "Windows 7 Nextion VBox" restore "Starting in 3 seconds..." 
rem "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" startvm "Windows 7 Nextion VBox" --type headless
rem copy ESP8266 binary to release folder. Usually no need for (re)compiling. 
robocopy Syntherrupter_Lightsaber Syntherrupter_Firmwares Syntherrupter_Lightsaber.ino.generic.bin
rem Compile Tiva Firmware for all possible output numbers.
for %%i in (1,2,3,4,5,6) do ( 
 C:\TI\ccsv8\eclipse\eclipsec -noSplash -data "C:\Users\Max\workspace_v8" -application com.ti.ccstudio.apps.projectBuild -ccs.projects Syntherrupter_Tiva -ccs.configuration Release_%%i
 robocopy Syntherrupter_Tiva\Release_%%i Syntherrupter_Firmwares Syntherrupter_Tiva.bin
 ren Syntherrupter_Firmwares\Syntherrupter_Tiva.bin Syntherrupter_Tiva_%%i_Coils.bin
 )
ren Syntherrupter_Firmwares\Syntherrupter_Tiva_1_Coils.bin Syntherrupter_Tiva_1_Coil.bin
rem Build VM takes at least 20s to start, and at least 20s to compile one file. We assume that compiling the 6 Tiva firmwares take at least 20s. So... if no HMI file is there, the VM didn't start yet, or has already finished. 
cd Syntherrupter_Firmwares
rem :waitforvm
rem timeout /t 5
rem if not exist "*.HMI" goto waitforvm
rem Build VM is configured to delete temporary HMI files after creation. wait until no HMI files are left.
rem :loop
timeout /t 5
rem if exist "*.HMI" goto loop
rem "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" controlvm "Windows 7 Nextion VBox" poweroff
rem "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" snapshot "Windows 7 Nextion VBox" restore "Starting in 3 seconds..."
"C:\Program Files\WinRAR\winrar" a -afzip -m5 ..\Syntherrupter_Firmwares.zip
cd ..
