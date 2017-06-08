file     = open("trace.log", 'r')
file_out = open("../tracefile/tracefile", 'w')

i = 0

while True:
    f = file.readline()
    log0  = f.split(" ")
    if 'NoC' in log0[0]:
        while True:
            if 'sc_time_stamp' in log0[0]:
                print("sc_time_stamp=",log0[2],log0[3])
                break
            file_out.write(f)
            f = file.readline()
            if f == '': break
            log0  = f.split(" ")
        break

        
#while i < 61:
#    f = file.readline()
#    i+=1
#
#for i in range(62,640062):
#    f = file.readline()
#    file_out.write(f)


file.close()
file_out.close()

file_out = open("../tracefile/tracefile", 'r')
PE0_trace = open("../tracefile/PE0/PE0_trace", 'w')
PE1_trace = open("../tracefile/PE1/PE1_trace", 'w')
PE2_trace = open("../tracefile/PE2/PE2_trace", 'w')
PE3_trace = open("../tracefile/PE3/PE3_trace", 'w')
PE4_trace = open("../tracefile/PE4/PE4_trace", 'w')
PE5_trace = open("../tracefile/PE5/PE5_trace", 'w')
PE6_trace = open("../tracefile/PE6/PE6_trace", 'w')
PE7_trace = open("../tracefile/PE7/PE7_trace", 'w')
PE8_trace = open("../tracefile/PE8/PE8_trace", 'w')
PE9_trace = open("../tracefile/PE9/PE9_trace", 'w')
PE10_trace = open("../tracefile/PE10/PE10_trace", 'w')
PE11_trace = open("../tracefile/PE11/PE11_trace", 'w')
PE12_trace = open("../tracefile/PE12/PE12_trace", 'w')
PE13_trace = open("../tracefile/PE13/PE13_trace", 'w')
PE14_trace = open("../tracefile/PE14/PE14_trace", 'w')
PE15_trace = open("../tracefile/PE15/PE15_trace", 'w')

while True:
    f = file_out.readline()
    if f == '': break
    log = f.split(" ")
    #print(f)
    if log[0] != 'NoC' and log[0] != 'CRO':
        continue;
    elif log[0] == 'Frame':
        continue;
    else:
        if log[7]=='0' or log[9]=='0':
            #print(log[6])
            PE0_trace.write(f)
        elif log[7]=='1' or log[9]=='1':
            PE1_trace.write(f)
        elif log[7]=='2' or log[9]=='2':
            PE2_trace.write(f)
        elif log[7]=='3' or log[9]=='3':
            PE3_trace.write(f)
        elif log[7]=='4' or log[9]=='4':
            PE4_trace.write(f)
        elif log[7]=='5' or log[9]=='5':
            PE5_trace.write(f)
        elif log[7]=='6' or log[9]=='6':
            PE6_trace.write(f)
        elif log[7]=='7' or log[9]=='7':
            PE7_trace.write(f)
        elif log[7]=='8' or log[9]=='8':
            PE8_trace.write(f)
        elif log[7]=='9' or log[9]=='9':
            PE9_trace.write(f)
        elif log[7]=='10' or log[9]=='10':
            PE10_trace.write(f)
        elif log[7]=='11' or log[9]=='11':
            PE11_trace.write(f)
        elif log[7]=='12' or log[9]=='12':
            PE12_trace.write(f)
        elif log[7]=='13' or log[9]=='13':
            PE13_trace.write(f)
        elif log[7]=='14' or log[9]=='14':
            PE14_trace.write(f)
        elif log[7]=='15' or log[9]=='15':
            PE15_trace.write(f)


PE0_trace.close()
PE1_trace.close()
PE2_trace.close()
PE3_trace.close()
PE4_trace.close()
PE5_trace.close()
PE6_trace.close()
PE7_trace.close()
PE8_trace.close()
PE9_trace.close()
PE10_trace.close()
PE11_trace.close()
PE12_trace.close()
PE13_trace.close()
PE14_trace.close()
PE15_trace.close()
file_out.close()


period = 4  #ns

for i in range(0,16):
    trace_file = "PE"+str(i)+"_trace"
    NoC_file = "PE"+str(i)+"_NoC"
    CRO_file = "PE"+str(i)+"_CRO"
    trace_file = open("../tracefile/PE"+str(i)+"/PE"+str(i)+"_trace", 'r')
    NoC_file = open("../tracefile/PE"+str(i)+"/PE"+str(i)+"_NoC", 'w')
    CRO_file = open("../tracefile/PE"+str(i)+"/PE"+str(i)+"_CRO", 'w')
    while True:
        f0 = trace_file.readline()
        if f0 == '': break
        log0  = f0.split(" ")
        if 'NoC' in log0[0]:
            NoC_file.write(f0)
        elif 'CRO' in log0[0]:
            CRO_file.write(f0)
    trace_file.close()
    NoC_file.close()
    CRO_file.close()

ToN_list = []
Bw_list  = []
    
avg_time = 0   
avg_cyc  = 0
   
start = 0
   
#for i in range(0,16):   
#    NoC_file = open("../tracefile/PE"+str(i)+"/PE"+str(i)+"_NoC", 'r')
#    total_time = 0
#    m = 0
#    n = 0
#    while True:
#        f0 = NoC_file.readline()
#        if f0 == '': break
#        log0 = f0.split(" ")
#        
#        if log0[12] == 'us':
#            time = int(log0[11])*1000
#        elif log0[12] == 'ms':
#            time = int(log0[11])*1000000
#        elif log0[12] == 'ps':
#            time = int(log0[11])/1000
#        else:
#            time = int(log0[11])
#        
#        
#        if log0[2] == 'ToN':
#            ToN_list.append(time)
#            m+=1
#        elif log0[2] == 'ToComm':
#            Bw_list.append(time)
#            n+=1
#    
#    for i in range(start,m+start):
#        total_time += Bw_list[i] - ToN_list[i]
#    
#    start = m
#    
#    avg_time += total_time/m
#    avg_cyc += ((total_time/m)/4)
#    
#    print(m,n,total_time,total_time/m, (total_time/m)/4)
#
#print(avg_time/16,avg_cyc/16)
    

#total_read = 0
#for i in range(0,16):
#    NoC_file = "PE"+str(i)+"_NoC"
#    CRO_file = "PE"+str(i)+"_CRO"
#    NoC_file = open("../tracefile/PE"+str(i)+"/PE"+str(i)+"_NoC", 'r')
#    CRO_file = open("../tracefile/PE"+str(i)+"/PE"+str(i)+"_CRO", 'r')
#    NoC_count = "Count_PE"+str(i)+"_NoC"
#    CRO_count = "Count_PE"+str(i)+"_CRO"
#    MEM_count = "Count_PE"+str(i)+"_MEM"
#    EACH_count = "Count_PE"+str(i)+"_EACH"
#    EACH_read = "Count_PE"+str(i)+"_EACH_read"
#    NoC_count_temp = 0
#    CRO_count_temp = 0
#    MEM_count_temp = 0
#    EACH_count_temp = 0
#    EACH_read_temp = 0
#    temp0 = 0
#    temp1 = 0
#    temp2 = 0
#    temp3 = 0
#    temp4 = 0
#    temp5 = 0
#    temp6 = 0
#    temp7 = 0
#    tempmem0 = 0
#    tempmem1 = 0
#    j = 1
#    read_num = 0
#    while True:
#        f0 = NoC_file.readline()
#        if f0 == '': break
#        log0 = f0.split(" ")
#        if j%2 == 1:
#            if 'us' in log0[12]:
#                temp0 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                temp0 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                temp0 = int(log0[11])/1000
#            else:
#                temp0 = int(log0[11])
#        elif j%2 == 0:
#            if 'us' in log0[12]:
#                temp1 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                temp1 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                temp1 = int(log0[11])/1000
#            else:
#                temp1 = int(log0[11])
#            NoC_count_temp += (temp1 - temp0)
#        
#        if j%4 == 1:
#            if 'us' in log0[12]:
#                temp6 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                temp6 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                temp6 = int(log0[11])/1000
#            else:
#                temp6 = int(log0[11])
#            EACH_read_temp += (temp6-temp7)
#        elif j%4 == 0:
#            if 'us' in log0[12]:
#                temp7 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                temp7 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                temp7 = int(log0[11])/1000
#            else:
#                temp7 = int(log0[11])
#            EACH_read_temp += (temp7-temp6)
#        
#        if 'FW' in log0[1] and 'ToN' in log0[2]:
#            if 'us' in log0[12]:
#                temp4 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                temp4 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                temp4 = int(log0[11])/1000
#            else:
#                temp4 = int(log0[11])
#        elif 'BW' in log0[1] and 'ToComm' in log0[2]:
#            if 'us' in log0[12]:
#                temp5 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                temp5 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                temp5 = int(log0[11])/1000
#            else:
#                temp5 = int(log0[11])
#            EACH_count_temp += (temp5 - temp4)
#            read_num+=1
#        
#        
#        f1 = CRO_file.readline()
#        if f1 == '': break
#        log0 = f1.strip("\n")
#        log0 = log0.split(" ")
#        if j%2 == 1:
#            if 'us' in log0[12]:
#                temp2 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                temp2 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                temp2 = int(log0[11])/1000
#            else:
#                temp2 = int(log0[11])
#        elif j%2 == 0:
#            if 'us' in log0[12]:
#                temp3 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                temp3 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                temp3 = int(log0[11])/1000
#            else:
#                temp3 = int(log0[11])
#            CRO_count_temp += (temp3 - temp2)
#        #print(temp1 - temp0)
#    
#        if 'FW' in log0[1] and 'ToM' in log0[2]:
#            if 'us' in log0[12]:
#                tempmem0 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                tempmem0 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                tempmem0 = int(log0[11])/1000
#            else:
#                tempmem0 = int(log0[11])
#        elif 'BW' in log0[1] and 'ToC' in log0[2]:
#            if 'us' in log0[12]:
#                tempmem1 = int(log0[11])*1000
#            elif 'ms' in log0[12]:
#                tempmem1 = int(log0[11])*1000000
#            elif 'ps' in log0[12]:
#                tempmem1 = int(log0[11])/1000
#            else:
#                tempmem1 = int(log0[11])
#            MEM_count_temp += (tempmem1-tempmem0)
#        #print(Count_PE0_MEM)
#        
#        j+=1
#    
#    #EACH_read_temp = (1/(EACH_read_temp/10000))*1000
#    total_read += read_num
#    if i < 10:
#        print(NoC_count, "", NoC_count_temp, CRO_count, "", CRO_count_temp, MEM_count, "",MEM_count_temp, EACH_count, "",EACH_count_temp, read_num)
#    else:
#        print(NoC_count, NoC_count_temp, CRO_count, CRO_count_temp, MEM_count, MEM_count_temp, EACH_count, EACH_count_temp, read_num)
#    #print(EACH_read, EACH_read_temp, "M/s")
#    #print(CRO_count, CRO_count_temp)
#    
#    #print(MEM_count, MEM_count_temp)
#    #print(EACH_count, EACH_count_temp)
#    
#    NoC_file.close()
#    CRO_file.close()

#print("Total_read:", total_read,) 
