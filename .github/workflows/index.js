const core = require('@actions/core');
const exec = require('@actions/exec');
const path = require('path');

async function run() {
  try {
    const arch = core.getInput('arch', { required: true });
    const vsVersion = core.getInput('vsversion');

    // Map input architecture to vcvarsall.bat arguments
    const archMap = {
      'x64': 'x64',
      'arm64': 'arm64',
      'x86': 'x86',
      'x64_arm64': 'x64_arm64', // Cross-compile ARM64 on x64
      'x64_x86': 'x64_x86',     // Cross-compile x86 on x64
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

    let vswhereArgs = ['-latest', '-products *', '-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property installationPath'];
    if (vsVersion) {
      vswhereArgs.push('-version', vsVersion);
    }

    let vswhereOutput = '';
    await exec.exec(`"${vswherePath}"`, vswhereArgs, {
      listeners: {
        stdout: (data) => {
          vswhereOutput += data.toString();
        }
      }
    });

    vsInstallPath = vswhereOutput.trim();
    if (!vsInstallPath) {
      core.setFailed('Could not find Visual Studio installation');
      return;
    }

    // Path to vcvarsall.bat
    const vcvarsPath = path.join(vsInstallPath, 'VC', 'Auxiliary', 'Build', 'vcvarsall.bat');

    // Create a temporary batch file to capture environment variables
    const tempBat = 'msvc-dev-cmd.bat';
    const batContent = `
      @echo off
      call "${vcvarsPath}" ${vcvarsArch}
      if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
      set
    `;

    require('fs').writeFileSync(tempBat, batContent);

    // Run the batch file and capture environment
    let envOutput = '';
    await exec.exec('cmd.exe', ['/q', '/c', tempBat], {
      listeners: {
        stdout: (data) => {
          envOutput += data.toString();
        }
      }
    });

    // Parse and set environment variables
    envOutput.split('\n').forEach(line => {
      const [key, value] = line.split('=', 2);
      if (key && value) {
        core.exportVariable(key.trim(), value.trim());
      }
    });

    // Ensure MSVC linker is prioritized over GNU tools
    const msLinkPath = path.join(vsInstallPath, 'VC', 'Tools', 'MSVC');
    const msLinkVersion = require('fs').readdirSync(msLinkPath)[0]; // Get latest MSVC version
    const binPath = path.join(msLinkPath, msLinkVersion, 'bin', `Host${vcvarsArch.includes('arm64') ? 'arm64' : 'x64'}`, vcvarsArch.includes('x86') ? 'x86' : vcvarsArch);
    core.addPath(binPath);

  } catch (error) {
    core.setFailed(`Action failed: ${error.message}`);
  }
}

run();