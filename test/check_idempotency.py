#!/usr/bin/env python3
import os
import subprocess
import sys
import re

def run_test(opt_debugger_bin, ir_file):
    print(f"[*] Testing structural idempotency: {ir_file}")
    cmd = [
        opt_debugger_bin,
        "--before=" + ir_file,
        "--after=" + ir_file,
        "--remarks=/dev/null"
    ]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        output = result.stdout
        
        
        passed = ("Modified   : 0" in output) or ("No optimization opportunities detected" in output)
        
        if passed:
            print("[+] PASS: Structural idempotency verified (0 changes).")
            return True
        else:
            print("[!] FAIL: Unexpected output format or hidden modifications.")
            print("--- OUTPUT START ---")
            print(output)
            print("--- OUTPUT END ---")
            return False
            
    except subprocess.CalledProcessError as e:
        print(f"[!] Binary crashed or returned error: {e}")
        print(e.stdout)
        print(e.stderr)
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: check_idempotency.py <bin> <ir_file>")
        sys.exit(1)
    
    if not run_test(sys.argv[1], sys.argv[2]):
        sys.exit(1)
