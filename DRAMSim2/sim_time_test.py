import os
'''
This program is to execute DRAMSim2 with specifiec simulation cycles
automatically. It execute DRAMSim2 in quiet mode, which will only print the
executed time of DRAMSim2 only.
The result will append to the file "sim_time.txt".
'''

for i in range(15, 15+11):
    sim_cycle = str(2**i)
    command = "./Test "
    command = command + " "  + sim_cycle + " >> sim_time.txt"
    print command
    os.system(command)
