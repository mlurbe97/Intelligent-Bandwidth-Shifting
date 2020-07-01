#!/bin/bash

helpFunction()
{
   echo ""
   echo "Usage: $0 -a workloadArray -b dirOut"
   echo -e "\t-a workloadArray benchmaks workload to be executed. Posibilities:"
   echo -e "\t\ta: 6 apps workloads"
   echo -e "\t\tb: 8 apps workloads"
   echo -e "\t\tc: 10 app workloads"
   echo -e "\t\td: custom workloads (edit script)."
   echo -e "\t-b dirOut is the output directory."
   exit 1 # Exit script after printing help
}

# Get the arguments
while getopts "a:b:c:" opt
do
   case "$opt" in
      a ) ARRAY="$OPTARG" ;;
      b ) DIROUT="$OPTARG" ;;
      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
   esac
done

# Print helpFunction in case parameters are empty
if [ -z "$ARRAY" ] || [ -z "$DIROUT" ]
then
   echo "Missing arguments.";
   helpFunction
fi

if mkdir outManel/${DIROUT} ; then
    echo \#Directory created working_dir/outManel/${DIROUT}\#
else
    echo ""
    echo \#Error creating directory working_dir/outManel/${DIROUT}\#
    echo \#The directory may already exist\#
    echo ""
fi


# Select the array of benckmarks to be executed
case "$ARRAY" in
   #case 1
   "a") workloadArray="2 3 5 6 7 8 9 10 11 12 13 16 17 18 20 22 23";;## 6 applications workload.
      
   #case 2 
   "b") repArray="26 27 28 29 30 31 32 33 34 35 36 37 38 39" ;;# 8 applications workload.

   #case 3 
   "c") workloadArray="40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55" ;;## 10 applications workload.

   #case 4 
   "d") workloadArray="26 27 28 29 30 31 32 33 34 35 36 37 38 39 42 43 44 45 46 47 48 49 50 51 52 53 54 55" ;;# Custom applications workload.

   #case 7 
   *) echo "Invalid entry for repArray."
      helpFunction;; 
esac

{ ./start_Experiments.sh; }

for workload in $workloadArray
do
      sudo ./IBS -A $workload -F -Q 2>> ${DIROUT}/trabajo[${workload}]conf[ibs].txt
done;

{ ./end_Experiments.sh; }
