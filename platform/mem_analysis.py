file_in_list = [
"../mem_trace/mem0",
"../mem_trace/mem1",
"../mem_trace/mem2",
"../mem_trace/mem3"
]

mem_cnt = 0
frequency = 4
time = 0

total_mem_cnt = 0
total_all_mem_time = 0
total_mem_time = 0

fw_list = []
raddr_list = []

fw_addr = []
raddr_addr = []


start = 0

for i in range(0,4):
    trace_file = open(file_in_list[i], 'r')
    mem_cnt = 0
    each_mem_time = 0
    each_mem_all_time = 0
    tmp_time = 0
    mem_time = 0
    mem_all_time = 0
    
    while True:
        f0 = trace_file.readline()
        if f0 == '': break
        log = f0.split(" ")
        
        if log[4] == 'us':
            time = int(log[3])*1000
        elif log[4] == 'ms':
            time = int(log[3])*1000000
        elif log[4] == 'ps':
            time = int(log[3])/1000
        else:
            time = int(log[3])
            
            
        if log[1] == 'fw_sc_time_stamp':
            fw_list.append(time)
            fw_addr.append(log[10])
        elif log[1] == 'radd_sc_time_stamp':
            raddr_list.append(time)
            raddr_addr.append(log[10])
        elif log[1] == 'read_complete_sc_time_stamp':
            index = raddr_addr.index(log[10])
            tmp_time = raddr_list[index]
            mem_time += (time - tmp_time)
           #print(time - tmp_time)
            raddr_list.remove(tmp_time)
            raddr_addr.remove(log[10])
        elif log[1] == 'rsp_sc_time_stamp':
            index = fw_addr.index(log[10])
            tmp_time = fw_list[index]
            mem_all_time += (time - tmp_time)
            fw_list.remove(tmp_time)
            fw_addr.remove(log[10])
            mem_cnt+=1
            
    total_mem_cnt += mem_cnt
    
    total_all_mem_time += mem_all_time
    total_mem_time += mem_time
    
    print(mem_cnt)
    print("mem"+str(i), (mem_time/mem_cnt)/frequency, (mem_all_time/mem_cnt)/frequency)

print((total_mem_time/total_mem_cnt)/frequency, (total_all_mem_time/total_mem_cnt)/frequency)        
            
            
            
            
            
            
            
