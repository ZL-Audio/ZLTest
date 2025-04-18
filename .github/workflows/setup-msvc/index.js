const core = require('@actions/core');
const exec = require('@actions/exec');
const path = require('path');
const fs = require('fs');

async function run() {
  try {
    const arch = core.getInput('arch', { required: true });
    const vsVersion = core.getInput('vsversion');

    // Map input architecture to vcvarsall.bat arguments
    const archMap = {
      'x64': 'x64',
      'arm64': 'arm64',
      'x86': 'x86',
      'x64_arm64': 'x64_arm64',
      'x64_x86': 'x64_x86',
      'amd64': 'x64',
      'amd64_arm64': 'x64_arm64',
      'amd64_x86': 'x64_x86',
      'Win32': 'x86',
      'Win64': 'x64'
    };

    if (!archMap[arch]) {
      core.setFailed(`Unsupported architecture: ${arch}`);
      return;
    }

    const vcvarsArch = archMap[arch];

    // Find Visual Studio installation using vswhere
    let vswherePath = 'C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe';
    let vsInstallPath = '';

    let vswhereArgs = ['-latest', '-products', '*', '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'];
    if (vsVersion) {
      vswhereArgs.push('-version', vsVersion);
    }

    let vswhereOutput = '';
    let vswhereError = '';
    core.debug(`Running vswhere with args: ${vswhereArgs.join(' ')}`);
    const vswhereExitCode = await exec.exec(`"${vswherePath}"`, vswhereArgs, {
      listeners: {
        stdout: (data) => {
          vswhereOutput += data.toString();
        },
        stderr: (data) => {
          vswhereError += data.toString();
        }
      },
      ignoreReturnCode: true
    });

    if (vswhereExitCode !== 0) {
      core.setFailed(`vswhere failed with exit code ${vswhereExitCode}: ${vswhereError}`);
      return;
    }

    vsInstallPath = vswhereOutput.trim();
    if (!vsInstallPath) {
      core.setFailed('Could not find Visual Studio installation');
      return;
    }

    core.debug(`Found Visual Studio installation at: ${vsInstallPath}`);

    // Path to vcvarsall.bat
    const vcvarsPath = path.join(vsInstallPath, 'VC', 'Auxiliary', 'Build', 'vcvarsall.bat');

    // Verify vcvarsall.bat exists
    if (!fs.existsSync(vcvarsPath)) {
      core.setFailed(`vcvarsall.bat not found at ${vcvarsPath}`);
      return;
    }

    // Create a temporary batch file to capture environment variables
    const tempBat = 'clang-cl-dev-cmd.bat';
    const batContent = `
      @echo off
      call "${vcvarsPath}" ${vcvarsArch}
      if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
      set
    `;

    fs.writeFileSync(tempBat, batContent);

    // Run the batch file and capture environment
    let envOutput = '';
    let envError = '';
    core.debug(`Running ${tempBat} to set up environment for ${vcvarsArch}`);
    const envExitCode = await exec.exec('cmd.exe', ['/q', '/c', tempBat], {
      listeners: {
        stdout: (data) => {
          envOutput += data.toString();
        },
        stderr: (data) => {
          envError += data.toString();
        }
      },
      ignoreReturnCode: true
    });

    if (envExitCode !== 0) {
      core.setFailed(`Failed to run vcvarsall.bat: ${envError}`);
      return;
    }

    // Parse and set environment variables
    envOutput.split('\n').forEach(line => {
      const [key, value] = line.split('=', 2);
      if (key && value) {
        core.exportVariable(key.trim(), value.trim());
      }
    });

    // Add LLVM bin directory to PATH for clang-cl.exe and lld-link.exe
    const llvmBinPath = 'C:\\Program Files\\LLVM\\bin';
    if (!fs.existsSync(llvmBinPath)) {
      core.setFailed(`LLVM bin directory not found at ${llvmBinPath}`);
      return;
    }
    core.addPath(llvmBinPath);
    core.debug(`Added LLVM bin path to PATH: ${llvmBinPath}`);

    // Set MSVC library path for lld-link.exe
    const msLinkPath = path.join(vsInstallPath, 'VC', 'Tools', 'MSVC');
    const msLinkVersions = fs.readdirSync(msLinkPath);
    if (msLinkVersions.length === 0) {
      core.setFailed('No MSVC toolset found in ' + msLinkPath);
      return;
    }
    const msLinkVersion = msLinkVersions[0]; // Get latest MSVC version
    const targetArch = vcvarsArch.includes('x86') ? 'x86' : vcvarsArch.includes('arm64') ? 'arm64' : 'x64';
    const libPath = path.join(msLinkPath, msLinkVersion, 'lib', targetArch);
    if (!fs.existsSync(libPath)) {
      core.setFailed(`MSVC library directory not found at ${libPath}`);
      return;
    }

    // Append to LIB environment variable
    const existingLib = process.env.LIB || '';
    const newLib = existingLib ? `${existingLib};${libPath}` : libPath;
    core.exportVariable('LIB', newLib);
    core.debug(`Set LIB to: ${newLib}`);

    // Set CMake compiler and linker variables
    core.exportVariable('CC', 'clang-cl.exe');
    core.exportVariable('CXX', 'clang-cl.exe');
    core.exportVariable('CMAKE_LINKER', 'lld-link.exe');

    // Set linker flags to include MSVC CRT (dynamic CRT for -MDd)
    const crtLib = 'msvcrtd.lib'; // Dynamic debug CRT for -MDd
    core.exportVariable('CMAKE_EXE_LINKER_FLAGS', `/LIBPATH:"${libPath}" ${crtLib}`);
    core.exportVariable('CMAKE_CXX_LINKER_FLAGS', `/LIBPATH:"${libPath}" ${crtLib}`);

  } catch (error) {
    core.setFailed(`Action failed: ${error.message}`);
  } finally {
    // Clean up temporary batch file
    if (fs.existsSync('clang-cl-dev-cmd.bat')) {
      fs.unlinkSync('clang-cl-dev-cmd.bat');
    }
  }
}

run();