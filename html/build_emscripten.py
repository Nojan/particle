import os, errno, copy
from subprocess import call, check_call

source_dir = os.path.normpath("../sources")
build_dir = os.path.normpath("../emscripten_build")
include_dir = ["-I" + os.path.normpath("../externals/glm"), "-I" + os.path.normpath("../externals")]
build_flag = ["-s", "USE_SDL=2", "-Os", "-msse2"]
link_flag = []#["-s", "WASM=1"]
preload_data = [os.path.normpath("../shaders")]

emcc = ["emcc"]
emcc.extend(build_flag)
empp = ["em++"]
empp.extend(build_flag)
empp.append("-std=c++11")

def ensure_file(filepath):
    """
    Ensure that a filepath exists; if it does not, attempt to create it.
    """
    dirname, filename = os.path.split(filepath) 
    try:
        os.makedirs(dirname)
    except OSError, e:
        if e.errno != errno.EEXIST:
            raise

def gatherSourceFile(source_dir, source_ext):
    file_list = list()
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.endswith(source_ext):
                filepath = os.path.join(root, file)
                file_list.append(filepath)
    return file_list

def buildBytecode(compiler, build_dir, file_list, file_ext):
    output_list = list()
    compiler_include =  copy.deepcopy(compiler)
    compiler_include.extend(include_dir)
    for file in file_list:
        input_file = file
        output_file = file.replace(source_dir, build_dir).replace(file_ext, ".bc")
        output_list.append(output_file)
        if os.path.isfile(output_file):
            input_file_timestep = os.path.getmtime(input_file);
            output_file_timestep = os.path.getmtime(output_file);
            if input_file_timestep < output_file_timestep:
                print output_file + " is up to date"
                continue;
        ensure_file(output_file)
        command = copy.deepcopy(compiler_include)
        command.extend([input_file, "-o", output_file])
        print " ".join(command)
        check_call(command, shell=True)
    return output_list

list_cpp = gatherSourceFile(source_dir, ".cpp")
list_c = gatherSourceFile(source_dir, ".c")
list_bytecode = list()

list_bytecode.extend( buildBytecode(empp, build_dir, list_cpp, ".cpp") )
list_bytecode.extend( buildBytecode(emcc, build_dir, list_c, ".c") )

link_command = copy.deepcopy(emcc)
for file in list_bytecode:
    link_command.append(file)
link_command.append("-o")
link_command.append("index.html")
for preload in preload_data:
    link_command.append("--preload-file")
    link_command.append(preload)
link_command.extend(link_flag)
print " ".join(link_command)
check_call(link_command, shell=True)
