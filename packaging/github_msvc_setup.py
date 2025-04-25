import os
import sys
import platform
import subprocess


def main():
    github_env = os.getenv("GITHUB_ENV", "")
    for year in range(2050, 2000, -1):
        msvc_path = 'C:/Program Files/Microsoft Visual Studio/{}/Enterprise/VC/Auxiliary/Build/vcvars64.bat'.format(year)
        msvc_path = os.path.abspath(msvc_path)
        if os.path.exists(msvc_path):
            print(f"Found MSVC at: {msvc_path}")
            cmd = f'cmd.exe /c ""{msvc_path}" && set"'
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Error running {msvc_path}: {result.stderr}")
                return False
            with open(github_env, 'wb') as f:
                for line in result.stdout.splitlines():
                    if '=' in line:
                        print(line)
                        f.write(line.encode())
            return

if __name__ == '__main__':
    sys.exit(main())
