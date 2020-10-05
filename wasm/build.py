import os, sys, subprocess
from subprocess import Popen

script_directory = os.path.abspath(sys.argv[1]);
root_directory = os.path.normpath(os.path.join(script_directory, '..'))

output_object = os.path.join(root_directory, 'bin', 'wasm', 'carnage3D.html')

sdk_root_path = os.environ['SDKDIR'];
print ('SDK: ' + sdk_root_path)

glm_root_path = os.path.join(sdk_root_path, 'GLM/include')

os.chdir(root_directory)
print ('Root: ' + root_directory)

args = ['em++']

def scan_sources(sub_path):
	full_path = os.path.join(root_directory, sub_path)
	for name in os.listdir(full_path):
		if name.endswith('.cpp'):
			args.append(os.path.join(sub_path, name))

scan_sources('src')
scan_sources('third_party/Box2D/Box2D/Collision/Shapes')
scan_sources('third_party/Box2D/Box2D/Collision')
scan_sources('third_party/Box2D/Box2D/Common')
scan_sources('third_party/Box2D/Box2D/Dynamics/Contacts')
scan_sources('third_party/Box2D/Box2D/Dynamics/Joints')
scan_sources('third_party/Box2D/Box2D/Dynamics')

#args.append('--preload-file data')
args.append('-o ' + output_object)
args.append('-std=c++11')
args.append('-s WASM=1')
args.append('-s USE_GLFW=3')
args.append('-s FULL_ES3=1')
args.append('-s EXIT_RUNTIME=1')
args.append('-I third_party/Box2D')
args.append('-I ' + glm_root_path)
args.append('--preload-file gamedata')

commandline = (' '.join(args))
print (commandline)

process = Popen(commandline, shell = True)
stdout, stderr = process.communicate()