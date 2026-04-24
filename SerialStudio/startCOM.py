import subprocess
import time
import os
import shlex
import json

simulation = False


file_dir = os.path.dirname(os.path.abspath(__file__))

def com0com(cmd):
    com0com_exec = os.path.abspath(file_dir + '/com0com/setupc.exe')

    proc = subprocess.Popen([com0com_exec, cmd], 
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            cwd=os.path.abspath(file_dir + '/com0com'))
    time.sleep(0.1)
    [out, err] = proc.communicate()
    proc.kill()
    return [out.decode('utf-8'), err.decode('utf-8')]

def com0com_clear():
    # Grab all open Port Pairs
    [portList, err] = com0com('list')
    if err != '':
        print(err)
    for line in portList.split('\n'):
        for el in line.split(' '):
            if el.__contains__('CNCA'):
                # and remove every open Port Pair
                com0com('remove '+ el[-1])

def serialStudio_testAPI(cmd):
    test_api_path = os.path.abspath(file_dir + '/test_api.py')
    proc = subprocess.Popen(['python', test_api_path] + shlex.split(cmd), 
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    time.sleep(0.1)
    [out, err] = proc.communicate()
    proc.kill()
    return [out.decode('utf-8'), err.decode('utf-8')]

processes = []

print('Starting com0com Setup...')

# Remove all open com0com port pairs
com0com_clear()

# Setup needed Port Pairs

# Parsed Data for SerialStudio is outputted to COM8 and
# used by SerialStudio on COM9
com0com('install PortName=COM8 PortName=COM9')
com0com('install PortName=COM10 PortName=COM11')
if simulation:
    # Data Simulator puts data on COM1
    # Data Parser reads data on COM2
    com0com('install PortName=COM1 PortName=COM2')
else:
    # USB2TTL puts data on COM6
    # Data Parser reads data on COM2
    com0com('install PortName=COM2,EmuBR=yes,EmuOverrun=yes PortName=CNCB3,EmuBR=yes,EmuOverrun=yes')
    processes.append(subprocess.Popen(['hub4com', '--route=All:All', '--baud=115200', '\\\\.\\COM6', '\\\\.\\CNCB3']))
    

time.sleep(0.5)

print('com0com Setup finished!')

time.sleep(1)

print('Starting Serial Studio...')

processes.append(subprocess.Popen(['Serial-Studio-GPL3', '--api-server'],
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE))
time.sleep(15)

print('Serial Studio started!')

print('Setting up Serial Studio...')

[portList_testAPI_OUT, err] = serialStudio_testAPI('send io.driver.uart.getPortList')
if err != '':
    print(err)

portList = json.loads(portList_testAPI_OUT)['portList']

for port in portList:
    if port['name'].split(' ')[0] == 'COM9':
        portIndex = port['index']

serialStudio_testAPI('send dashboard.setOperationMode -p mode=0')
serialStudio_testAPI('send io.manager.setBusType -p busType=0')
serialStudio_testAPI(f'send io.driver.uart.setPortIndex -p portIndex={portIndex}')
serialStudio_testAPI('send io.driver.uart.setBaudRate -p baudRate=115200')
serialStudio_testAPI('send io.manager.connect')

print('Setup completed!')

print('Start PuTTY...')

processes.append(subprocess.Popen(['putty', '-serial', 'COM11', '-sercfg', '115200,8,n,1,N']))

print('PuTTY started')

print('Starting Python Scripts...')

processes.append(subprocess.Popen(['python', file_dir + '/comParser.py']))
if simulation:
    processes.append(subprocess.Popen(['python', file_dir + '/comSimulation.py']))


try:
    input('Press Enter to stop...')
finally:
    serialStudio_testAPI('send io.manager.disconnect')
    for p in processes:
        p.terminate()
    com0com_clear()
    print("All processes stopped.")