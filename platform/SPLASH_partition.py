
FW_InitNI_timelist = []
FW_InitNI_addrlist = []
BW_InitNI_timelist = []
BW_InitNI_addrlist = []

MEM_timelist = []
MEM_addrlist = []

FW_EACH_read_time = []
FW_EACH_read_addr = []


total_NoC_time = 0
total_MEM_time = 0
total_each_time = 0
total_read_times = 0

freq = 4
mem_freq = 4


for i in range(0,16):
    NoC_file = open("../tracefile/PE"+str(i)+"/PE"+str(i)+"_NoC", 'r')
    CRO_file = open("../tracefile/PE"+str(i)+"/PE"+str(i)+"_CRO", 'r')
    NoC_time = 0
    MEM_time = 0
    read_times = 0
    each_read_times = 0
    
    while True:
        f0 = NoC_file.readline()
        if f0 == "": break
        f0 = f0.strip("\n")
        log0 = f0.split(" ")
        
        if log0[12] == 'us':
            time = int(log0[11])*1000
        elif log0[12] == 'ms':
            time = int(log0[11])*1000000
        elif log0[12] == 'ps':
            time = int(log0[11])/1000
        else:
            time = int(log0[11])
    
        if log0[1] == "FW" and log0[2] == "ToN":
            FW_InitNI_timelist.append(time)
            FW_InitNI_addrlist.append(log0[13])
            
            if log0[13][4] == "0" and log0[13][5] == "0" and log0[13][6] == "0" and log0[13][7] == "0" and log0[13][8] == "0" and log0[13][9] == "0":
                tmp_word = log0[13][0:2]+"0"
            elif log0[13][4] == "0" and log0[13][5] == "0" and log0[13][6] == "0" and log0[13][7] == "0" and log0[13][8] == "0":
                tmp_word = log0[13][0:2]+log0[13][9:10]
            elif log0[13][4] == "0" and log0[13][5] == "0" and log0[13][6] == "0" and log0[13][7] == "0":
                tmp_word = log0[13][0:2]+log0[13][8:10]
            elif log0[13][4] == "0" and log0[13][5] == "0" and log0[13][6] == "0":
                tmp_word = log0[13][0:2]+log0[13][7:10]
            elif log0[13][4] == "0" and log0[13][5] == "0":
                tmp_word = log0[13][0:2]+log0[13][6:10]
            elif log0[13][4] == "0":
                tmp_word = log0[13][0:2]+log0[13][5:10]
            else:
                tmp_word = log0[13][0:2]+log0[13][4:10]          
            
            FW_EACH_read_time.append(time)
            FW_EACH_read_addr.append(tmp_word)
        elif log0[1] == "BW" and log0[2] == "ToN":
            BW_InitNI_timelist.append(time)
            BW_InitNI_addrlist.append(log0[13])
        elif log0[1] == "FW" and log0[2] == "ToC":
            index = FW_InitNI_addrlist.index(log0[13])
            tmp_time = FW_InitNI_timelist[index]
            NoC_time += (time - tmp_time)
            FW_InitNI_addrlist.remove(log0[13])
            FW_InitNI_timelist.remove(tmp_time)
        elif log0[1] == "BW" and log0[2] == "ToComm":
            index = BW_InitNI_addrlist.index(log0[13])
            tmp_time = BW_InitNI_timelist[index]
            NoC_time += (time - tmp_time)
            BW_InitNI_addrlist.remove(log0[13])
            BW_InitNI_timelist.remove(tmp_time)
            
            index2 = FW_EACH_read_addr.index(log0[13])
            tmp2_time = FW_EACH_read_time[index2]
            each_read_times += (time - tmp2_time)
            FW_EACH_read_addr.remove(log0[13])
            FW_EACH_read_time.remove(tmp2_time)
            read_times += 1

 
    while True:    
        f1 = CRO_file.readline()    
        if f1 == "": break
        f1 = f1.strip("\n")
        log1 = f1.split(" ")
        
        if log1[12] == 'us':
            time = int(log1[11])*1000
        elif log1[12] == 'ms':
            time = int(log1[11])*1000000
        elif log1[12] == 'ps':
            time = int(log1[11])/1000
        else:
            time = int(log1[11])
         

        if log1[1] == "FW" and log1[2] == "ToM":
            MEM_timelist.append(time)
            if log1[13][4] == "0" and log1[13][5] == "0" and log1[13][6] == "0" and log1[13][7] == "0" and log1[13][8] == "0" and log1[13][9] == "0":
                MEM_addrlist.append(log1[13][0:2]+"0")
            elif log1[13][4] == "0" and log1[13][5] == "0" and log1[13][6] == "0" and log1[13][7] == "0" and log1[13][8] == "0":
                MEM_addrlist.append(log1[13][0:2]+log1[13][9:10])
            elif log1[13][4] == "0" and log1[13][5] == "0" and log1[13][6] == "0" and log1[13][7] == "0":
                MEM_addrlist.append(log1[13][0:2]+log1[13][8:10])
            elif log1[13][4] == "0" and log1[13][5] == "0" and log1[13][6] == "0":
                MEM_addrlist.append(log1[13][0:2]+log1[13][7:10])
            elif log1[13][4] == "0" and log1[13][5] == "0":
                MEM_addrlist.append(log1[13][0:2]+log1[13][6:10])  
            elif log1[13][4] == "0":
                MEM_addrlist.append(log1[13][0:2]+log1[13][5:10])
            else:
                MEM_addrlist.append(log1[13][0:2]+log1[13][4:10])
        elif log1[1] == "BW" and log1[2] == "ToC":
            index = MEM_addrlist.index(log1[13])
            tmp_time = MEM_timelist[index]
            MEM_time += (time - tmp_time)
            MEM_addrlist.remove(log1[13])
            MEM_timelist.remove(tmp_time)        
    
    total_NoC_time += NoC_time
    total_MEM_time += MEM_time
    total_each_time += each_read_times
    total_read_times += read_times

    if read_times != 0:
        print(read_times, (NoC_time/read_times)/freq, (MEM_time/read_times)/mem_freq, (each_read_times/read_times)/freq)        
#        total_NoC_time += (NoC_time/read_times)/freq
#        total_MEM_time += (MEM_time/read_times)/mem_freq
#        total_each_time += (each_read_times/read_times)/freq
    else:
       print(0,0,0)        
            
print((total_NoC_time/total_read_times)/freq, (total_MEM_time/total_read_times)/mem_freq, (total_each_time/total_read_times)/freq)
#print(total_NoC_time/16, total_MEM_time/16, total_each_time/16)
            
            
