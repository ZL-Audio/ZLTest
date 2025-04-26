import os
import sys
import platform
import subprocess

def get_bat_name():
    arch = platform.machine().lower()
    if arch in ('x86_64', 'amd64'):
        return 'vcvars64.bat'
    elif arch in ('arm64', 'aarch64'):
        return 'vcvarsarm64.bat'
    else:
        return ''


def main():
    github_env = os.getenv("GITHUB_ENV", "")
    bat_name = get_bat_name()
    for year in range(2022, 2099):
        msvc_path = 'C:/Program Files/Microsoft Visual Studio/{}/Enterprise/VC/Auxiliary/Build/{}'.format(year, bat_name)
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
                        line += '\n'
                        f.write(line.encode())
            return

if __name__ == '__main__':
    sys.exit(main())
