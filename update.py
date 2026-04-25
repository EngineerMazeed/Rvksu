#!/usr/bin/env python3

import subprocess
import os
import sys
from datetime import datetime

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LOG_FILE = os.path.join(SCRIPT_DIR, f"fw_setup_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log")

NOTE = """
-Rvksu
This script is intended for research and debugging on specific Linux environments.
Logs are recorded for testing and development purposes.

WARNING: This script is specifically configured for Ubuntu 20.04 (Focal). 
Do not attempt to run it on newer OS versions or different distributions, 
as it will likely break the system or cause instability.
"""

def run(cmd):
    print(f"\n[+] {cmd}\n")
    process = subprocess.Popen(
        cmd, shell=True, stdout=subprocess.PIPE, 
        stderr=subprocess.STDOUT, text=True, bufsize=1
    )

    with open(LOG_FILE, "a") as log:
        for line in process.stdout:
            print(line, end="")
            log.write(f"[{datetime.now().strftime('%H:%M:%S')}] {line}")

    process.wait()
    return process.returncode

def safe_run(cmd):
    rc = run(cmd)
    if rc != 0:
        print(f"[!] Failed: {cmd}")

def fix_system():
    safe_run("rm -f /etc/apt/sources.list.d/google-chrome.list")
    safe_run("rm -f /etc/apt/sources.list.d/*.save")
    safe_run("apt clean && apt autoclean")
    safe_run("apt --fix-broken install -y")

    # Specifically setting Focal mirrors
    safe_run('bash -c \'echo "deb http://archive.ubuntu.com/ubuntu focal main universe multiverse restricted" > /etc/apt/sources.list\'')
    safe_run('bash -c \'echo "deb http://archive.ubuntu.com/ubuntu focal-updates main universe multiverse restricted" >> /etc/apt/sources.list\'')
    safe_run('bash -c \'echo "deb http://archive.ubuntu.com/ubuntu focal-security main universe multiverse restricted" >> /etc/apt/sources.list\'')
    safe_run("apt update -y")

def main():
    if os.geteuid() != 0:
        print("Root privileges required. Please run with sudo.")
        sys.exit(1)

    with open(LOG_FILE, "w") as log:
        log.write(NOTE + "\n")

    print(NOTE)

    fix_system()
    safe_run("apt upgrade -y")

    # Core system tools
    safe_run("apt install -y build-essential git wget curl unzip xz-utils p7zip-full")
    safe_run("apt install -y python3 python3-pip python3-dev liblzma-dev liblzo2-dev liblzo2-2")
    safe_run("apt install -y binwalk squashfs-tools squashfs-tools-ng mtd-utils gzip bzip2 tar lzop")
    safe_run("apt install -y cramfsprogs jffs2-utils")

    # Python environment
    safe_run("pip3 install --upgrade pip setuptools wheel")
    safe_run("pip3 install --user ubi_reader")
    safe_run("pip3 install jefferson python-lzo capstone pycryptodome")

    # Build tools from source
    os.chdir(SCRIPT_DIR)
    if not os.path.exists("firmware-mod-kit"):
        safe_run("git clone https://github.com/rampageX/firmware-mod-kit.git")

    if not os.path.exists("sasquatch"):
        safe_run("git clone https://github.com/devttys0/sasquatch.git")
        os.chdir("sasquatch")
        safe_run("./build.sh")
        os.chdir(SCRIPT_DIR)

    print("\n[✓] Env is ready")

if __name__ == "__main__":
    main()
