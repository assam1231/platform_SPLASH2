file = open("mem_trace.log", 'r')

mem0_trace = open("../mem_trace/mem0", 'w')
mem1_trace = open("../mem_trace/mem1", 'w')
mem2_trace = open("../mem_trace/mem2", 'w')
mem3_trace = open("../mem_trace/mem3", 'w')

while True:
    f = file.readline()
    if f == '': break
    log = f.split(" ")
    if log[0] == 'memory_0:':
        mem0_trace.write(f)
    elif log[0] == 'memory_1:':
        mem1_trace.write(f)
    elif log[0] == 'memory_2:':
        mem2_trace.write(f)
    elif log[0] == 'memory_3:':
        mem3_trace.write(f)
        
mem0_trace.close()
mem1_trace.close()
mem2_trace.close()
mem3_trace.close()