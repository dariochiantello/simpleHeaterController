### Simple heater controller

Compile using GCC (Linux Ubuntu): `gcc app.c getSettings.c -lm -o app`

./app to run the app

 -h help  
 -d [0] run normal mode  
 -d [1] run debug mode  
 -t [sec] set heater cycle in seconds (default 2.5)  

Arbitrary length of the control sequence up to max buffer size. 

Log file automatically generated when starting the app in debug mode.

Press d to toggle debug during the execution.  
Press q to quit.


Test routine to be implemented based on the log file.   
cmake file to be added


