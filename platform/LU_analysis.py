file_in_list = [
"./log/LU_PE0.log"         ,
"./log/LU_PE1.log"         ,
"./log/LU_PE2.log"         ,
"./log/LU_PE3.log"         ,
"./log/LU_PE4.log"         ,
"./log/LU_PE5.log"         ,
"./log/LU_PE6.log"         ,
"./log/LU_PE7.log"         ,
"./log/LU_PE8.log"         ,
"./log/LU_PE9.log"         ,
"./log/LU_PE10.log"        ,
"./log/LU_PE11.log"        ,
"./log/LU_PE12.log"        ,
"./log/LU_PE13.log"        ,
"./log/LU_PE14.log"        ,
"./log/LU_PE15.log"
]

total_read_request_time = 0
average_read_request_time = 0
average_read_request_cycle = 0

total_read_times = 2768277
PE0_read_times = 187684
PE1_read_times = 161857
PE2_read_times = 207342
PE3_read_times = 141936
PE4_read_times = 191938
PE5_read_times = 181917
PE6_read_times = 230347
PE7_read_times = 186814
PE8_read_times = 148477
PE9_read_times = 136736
PE10_read_times = 212820
PE11_read_times = 147868
PE12_read_times = 140988
PE13_read_times = 134876
PE14_read_times = 216081
PE15_read_times = 140596

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
    print(each_PE, each_PE/each_read, (each_PE/each_read)/frequency)
        
average_read_request_time = total_read_request_time / total
average_read_request_cycle = average_read_request_time / frequency
print(average_read_request_time, average_read_request_cycle, total)
            
            
