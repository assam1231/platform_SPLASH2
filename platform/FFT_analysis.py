file_in_list = [
"./log/FFT_PE0.log"         ,
"./log/FFT_PE1.log"         ,
"./log/FFT_PE2.log"         ,
"./log/FFT_PE3.log"         ,
"./log/FFT_PE4.log"         ,
"./log/FFT_PE5.log"         ,
"./log/FFT_PE6.log"         ,
"./log/FFT_PE7.log"         ,
"./log/FFT_PE8.log"         ,
"./log/FFT_PE9.log"         ,
"./log/FFT_PE10.log"        ,
"./log/FFT_PE11.log"        ,
"./log/FFT_PE12.log"        ,
"./log/FFT_PE13.log"        ,
"./log/FFT_PE14.log"        ,
"./log/FFT_PE15.log"
]

total_read_request_time = 0
average_read_request_time = 0
average_read_request_cycle = 0

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
            
            
