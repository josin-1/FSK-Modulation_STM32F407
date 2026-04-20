import subprocess
import os

dir = os.path.dirname(os.path.abspath(__file__))

processes = [
    subprocess.Popen(['hub4com', '--route=All:All', '\\\\.\\CNCB0', '\\\\.\\CNCB1']),
    subprocess.Popen(['hub4com', '--route=All:All', '\\\\.\\CNCB2', '\\\\.\\CNCB3']),
    subprocess.Popen(['hub4com', '--route=All:All', '\\\\.\\CNCB4', '\\\\.\\CNCB5']),
    subprocess.Popen(['python', dir + '/comParser.py']),
    subprocess.Popen(['python', dir + '/comSimulation.py']),
]

try:
    input("Press ENTER to stop all processes...\n")
finally:
    for p in processes:
        p.terminate()
    print("All processes stopped.")