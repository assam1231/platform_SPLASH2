file_in_list = [
"./log/rand_PE0.log"         ,
"./log/rand_PE1.log"         ,
"./log/rand_PE2.log"         ,
"./log/rand_PE3.log"         ,
"./log/rand_PE4.log"         ,
"./log/rand_PE5.log"         ,
"./log/rand_PE6.log"         ,
"./log/rand_PE7.log"         ,
"./log/rand_PE8.log"         ,
"./log/rand_PE9.log"         ,
"./log/rand_PE10.log"        ,
"./log/rand_PE11.log"        ,
"./log/rand_PE12.log"        ,
"./log/rand_PE13.log"        ,
"./log/rand_PE14.log"        ,
"./log/rand_PE15.log"
]

total_read_request_time = 0
average_read_request_time = 0
average_read_request_cycle = 0
total_read_times = 937069
PE0_read_times = 41622
PE1_read_times = 55266
PE2_read_times = 63753
PE3_read_times = 61112
PE4_read_times = 61396
PE5_read_times = 73738
PE6_read_times = 87270
PE7_read_times = 61964
PE8_read_times = 43250
PE9_read_times = 41764
PE10_read_times = 57818
PE11_read_times = 51185
PE12_read_times = 53244
PE13_read_times = 54369
PE14_read_times = 61311
PE15_read_times = 68007

total = 0
frequency = 4


for i in range(0,16):
    trace_file = open(file_in_list[i], 'r')
    EACH_PE_count = "Count_PE"+str(i)+"_EACH"
    read_time = "PE"+str(i)+"_read_times"
    each_PE = 0
    each_read = 0
    while True:
        f0 = trace_file.readline()
        if f0 == '': break
        log0 = f0.split(" ")
        if 'us' in log0[5]:
            time = int(log0[4])*1000
        elif 'ms' in log0[5]:
            time = int(log0[4])*1000000
        elif 'ps' in log0[5]:
            time = int(log0[4])/1000
        else:
            time = int(log0[4])
            
        each_PE += time
        each_read += 1
        total += 1
    total_read_request_time += each_PE
    
    if each_read != 0:
        print(each_PE, each_PE/each_read, (each_PE/each_read)/4)
    else:
        print(0,0,0)    
    
average_read_request_time = total_read_request_time / total
average_read_request_cycle = average_read_request_time/frequency
print(average_read_request_time, average_read_request_cycle, total)
            
            
