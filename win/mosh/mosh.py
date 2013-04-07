import subprocess
import socket
import sys
import os
env = os.environ.copy()
assert len(sys.argv) == 4
ip = socket.gethostbyname(sys.argv[1])
port = sys.argv[2]
key = sys.argv[3]
env["MOSH_KEY"]=key
subprocess.call(["mosh-client.exe", ip, port], env=env)
