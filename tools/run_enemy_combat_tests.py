"""Compile and run EnemyCombatTree transition tests with MSVC."""
from pathlib import Path
import os
import subprocess

root = Path(__file__).resolve().parent.parent
output = root / 'x64' / 'EnemyCombatTests'
output.mkdir(parents=True, exist_ok=True)
vcvars = Path(os.environ.get('ProgramFiles', 'C:/Program Files')) / 'Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build/vcvars64.bat'
args = ['cl', '/nologo', '/std:c++20', '/EHsc', '/utf-8',
        '/I' + str(root / 'Source'),
        str(root / 'tools/enemy_combat_tests.cpp'),
        str(root / 'Source/Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_tree.cpp'),
        '/Fe:' + str(output / 'enemy_combat_tests.exe')]
command = 'call "' + str(vcvars) + '" >nul && ' + subprocess.list2cmdline(args)
env = {key.upper(): value for key, value in os.environ.items()}
subprocess.run('cmd.exe /d /s /c "' + command + '"', cwd=output, env=env, check=True)
subprocess.run([str(output / 'enemy_combat_tests.exe')], cwd=root, env=env, check=True)
