#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys

arguments =  sys.argv
numargs = len(sys.argv)

if numargs != 3:
    print("Missing arguments")
    print("Usage: ./instructionsToCArray.py Time workloadArray [a->2006 b->2017 c->2006&2017]")
    sys.exit(1)

time = arguments[1]
workloadArray = arguments[2]

if workloadArray == 'a':
   benchmarks = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27]
elif workloadArray == 'b':
   benchmarks = [30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51]
elif workloadArray =='c':
    benchmarks = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51]
listaInstructions=[]

for bench in benchmarks:
    existe = 0
    try:
        fitx = open("instructions["+time+"]/Instructions["+str(bench)+"].txt","r")
        for linea in fitx.read().split("\n"):
            if "PMU_COUNTS:" in linea:
                existe = 1
                linea = linea.replace(" ","")
                instructions = linea.split("\t")[2]
                listaInstructions.append(instructions)
        if existe == 0:
            #print("El benchmark "+str(bench)+" no se ha podido contabilizar.\n")
            listaInstructions.append(str(0))
    except:
        listaInstructions.append(str(0))

escribir = open("instructions["+time+"]/array.txt","w")
escribir.write("unsigned long int instruccions_totals [] = {\n")

not_working = open("instructions["+time+"]/not_working.txt","w")
not_working.write("Benchmarks not working\n")

working = open("instructions["+time+"]/working.txt","w")
working.write("Benchmarks working\n")


cont=0
for value in listaInstructions:
    if value == '0':
        not_working.write(str(cont)+" ")
    else:
        working.write(str(cont)+" ")

    if cont != len(listaInstructions)-1:
        escribir.write(value+",")
    else:
        escribir.write(value)
    cont=cont+1
escribir.write("\n}")
escribir.close()
not_working.close()
working.close()