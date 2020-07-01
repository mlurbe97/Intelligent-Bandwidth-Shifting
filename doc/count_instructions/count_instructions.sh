#!/bin/bash

helpFunction()
{
   echo ""
   echo "usage: sudo ./count_instructions.sh -a time [Number in seconds] -b workloadArray [a->2006 b->2017]"
   exit 1 # Exit script after printing help
}

# Get the arguments
while getopts "a:b:" opt
do
   case "$opt" in
      a ) TIME="$OPTARG" ;;
      b ) ARRAY="$OPTARG" ;;
      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
   esac
done

# Print helpFunction in case parameters are empty
if [ -z "$TIME" ] || [ -z "$ARRAY" ]
then
   echo "Missing arguments.";
   helpFunction
fi

# Select the array of benckmarks to be executed
case "$ARRAY" in 

    #case 1 
    "a") workloadArray="1 2 3 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24" ;;## SPEC 2006 todos, el 4, el 25 y el 26 no funcionan (Ubuntu 18.04).

    #case 4 
    "b") workloadArray="31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51" ;;# SPEC 2017 todos, el 30 no funciona.

    #case 5 
    *) echo "Invalid entry for array of benckmarks."
      helpFunction;; 
esac

# Begin script in case all parameters are correct
{ ./startExperiments_Script.sh; }

for workload in $workloadArray
do
	./count_instructions_2017 -t $TIME -A $workload 2>> /home/malursem/working_dir/outManel/instructions[${TIME}]/Instructions[${workload}].txt
done;

{ ./endExperiments_Script.sh; }