import sys 

argvs = sys.argv  
argc = len(argvs) 

f = open(argvs[1])

line = f.readline() 

while line:

        tmp = line.split(",")
        print tmp[0].strip() #+ ",(" + str(tmp[1]).strip() + "%)
        line = f.readline()

    
