"""
Filename: run_client.py
Author: Ondrej Patejdl
Contact: opatejdl@students.zcu.cz
Description: This script defines starting script of application
    - create .venv and imports all needed dependencies
"""

import os
import sys
import subprocess
import venv

def setup_and_run():
    project_dir = os.path.dirname(os.path.abspath(__file__))
    venv_dir = os.path.join(project_dir, ".venv")
    req_file = os.path.join(project_dir, "pack_requirements.txt")

    main_script = os.path.join(project_dir, "src", "client.py")

    if sys.platform == "win32":
        venv_python = os.path.join(venv_dir, "Scripts", "python.exe")
        venv_pip = os.path.join(venv_dir, "Scripts", "pip.exe")
    else:
        venv_python = os.path.join(venv_dir, "bin", "python")
        venv_pip = os.path.join(venv_dir, "bin", "pip")

    if not os.path.exists(venv_dir):
        print(f"--- Creating venv and installing requirements...")
        venv.create(venv_dir, with_pip=True)
        if os.path.exists(req_file):
            subprocess.check_call([venv_pip, "install", "-r", req_file])

    if os.path.exists(main_script):
        print(f"--- Starting client from src/ folder...")

        env = os.environ.copy()
        env["PYTHONPATH"] = project_dir

        subprocess.run(
            [venv_python, main_script], 
            cwd=project_dir,
            env=env
        )
    else:
        print(f"!!! Error: {main_script} not found.")

if __name__ == "__main__":
    setup_and_run()