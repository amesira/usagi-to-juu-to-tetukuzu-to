"""Compile and run the real collision sources with MSVC (Windows)."""
from pathlib import Path
import os
import subprocess

root = Path(__file__).resolve().parent.parent
output = root / 'x64' / 'CapsuleTests'
output.mkdir(parents=True, exist_ok=True)
vcvars = Path(os.environ.get('ProgramFiles', 'C:/Program Files')) / 'Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build/vcvars64.bat'
sources = ['tools/capsule_collision_tests.cpp'] + [
    'Source/Engine/Processor/PhysicsPass/Collision/' + name + '.cpp'
    for name in ('collision_shape', 'overlap_test', 'raycast_test', 'sweep_test')]
args = ['cl', '/nologo', '/std:c++20', '/EHsc', '/utf-8', '/MD', '/Od',
        '/I' + str(root / 'Source'), '/I' + str(root / 'External'), '/I' + str(root),
        '/Fe:' + str(output / 'capsule_tests.exe')]
args += [str(root / source) for source in sources]
args += ['/link', '/LIBPATH:' + str(root)]
command = 'call "' + str(vcvars) + '" >nul && ' + subprocess.list2cmdline(args)
env = {key.upper(): value for key, value in os.environ.items()}
subprocess.run('cmd.exe /d /s /c "' + command + '"', cwd=output, env=env, check=True)
subprocess.run([str(output / 'capsule_tests.exe')], cwd=root, env=env, check=True)
