#Bash file for SleepingTa.c program
rm *.o
rm *.out

gcc -c -Wall -m64 -no-pie -o run.o SleepingTa.c -std=c2x
gcc -m64 -no-pie -o run.out run.o -lm -std=c2x      
#Make sure the executable has permission to execute
chmod u+x run.out
./run.out
