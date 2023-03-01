import os

# PREFIX = "prefix"
# AddOption('--prefix',dest='prefix',type='string', nargs=1, default='install',
    # action='store', metavar='DIR', help='installation prefix')

# env = Environment(tools=['default', 'protoc', 'grpc'])
# def_env = DefaultEnvironment(PREFIX = GetOption('prefix'))

# try:
#     env.Append(ENV = {'TERM' : os.environ['TERM']}) # Keep our nice terminal environment (like colors ...)
# except:
#     print("Not running in a terminal")

env=Environment(ENV=os.environ, tools=['default', 'protoc', 'grpc'])

print(env['ENV']['HOME'])
print(env['ENV']['PATH'])

if 'CC' in os.environ:
    env['CC']=os.environ['CC']
    
if 'CXX' in os.environ:
    env['CXX']=os.environ['CXX']

root_dir = Dir('#').srcnode().abspath
#
config = {}
# config['sse_root_dir'] = Dir('#/..').srcnode().abspath
#
config['cryto_lib_dir'] = root_dir + "/third_party/crypto/library"
config['cryto_include'] = config['cryto_lib_dir']  + "/include"
config['cryto_lib'] = config['cryto_lib_dir']  + "/lib"

config['db-parser_lib_dir'] = root_dir + "/third_party/db-parser/library"
config['db-parser_include'] = config['db-parser_lib_dir']  + "/include"
config['db-parser_lib'] = config['db-parser_lib_dir']  + "/lib"



if 'config_file' in ARGUMENTS:
    SConscript(ARGUMENTS['config_file'], exports=['env','config'])



env.Append(CCFLAGS = ['-fPIC','-Wall', '-march=native'])
env.Append(CXXFLAGS = ['-std=c++14'])
env.Append(CPPPATH = ['/usr/local/include', config['cryto_include'], config['db-parser_include'],'/usr/local/include/cryptopp'])
env.Append(LIBPATH = ['/usr/local/lib', config['cryto_lib'], config['db-parser_lib']])
env.Append(RPATH = [config['cryto_lib'], config['db-parser_lib']])

env.Append(LIBS = ['crypto', 'sse_crypto', 'grpc++_unsecure', 'grpc', 'protobuf', 'pthread', 'dl', 'sse_dbparser', 'rocksdb', 'snappy', 'z', 'bz2',  'lz4', 'cryptopp', 'glog'])

#Workaround for OS X
if env['PLATFORM'] == 'darwin':
    rpathprefix = '-rpath'
    env.Append(LINKFLAGS = [[rpathprefix, lib] for lib in env['RPATH']])
    env.Append(CPPPATH=['/usr/local/opt/openssl/include'])
    env.Append(LIBPATH=['/usr/local/opt/openssl/lib'])    
    # env.Append(LINKFLAGS = ['-rpath', cryto_lib_dir+'/lib'])



env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1


debug = ARGUMENTS.get('debug', 0)
if int(debug):
	env.Append(CCFLAGS = ['-O3'])
else:
	env.Append(CCFLAGS = ['-g', '-O0'])

static_relic = ARGUMENTS.get('static_relic', 0)

env.Append(CPPDEFINES = ['BENCHMARK'])

def run_test(target, source, env):
    app = str(source[0].abspath)
    if os.spawnl(os.P_WAIT, app, app)==0:
        return 0
    else:
        return 1

bld = Builder(action = run_test)
env.Append(BUILDERS = {'Test' :  bld})


crypto_lib_target = env.Command(config['cryto_lib_dir'], "", "cd third_party/crypto && scons lib")
db_parser_target = env.Command(config['db-parser_lib_dir'], "", "cd third_party/db-parser && scons lib")
env.Alias('deps', [crypto_lib_target, db_parser_target])

objects = SConscript('src/build.scons', exports='env', variant_dir='build')
# protos = SConscript('src/protos/build.scons', exports='env', duplicate=0)
# Depends(objects, protos)


env.Depends(objects["orion"],[crypto_lib_target , db_parser_target])


# clean_crypto = env.Command("clean_crypto", "", "cd third_party/crypto && scons -c lib")
# env.Alias('clean_deps', [clean_crypto])

Clean(objects["orion"], 'build')

outter_env = env.Clone()
outter_env.Append(CPPPATH = ['build'])


#orion_debug_prog   = outter_env.Program('orion_debug',    ['test_orion.cpp']     + objects["orion"])
client_debug_prog   = outter_env.Program('test_omap',    ['test_omap.cpp']     + objects["orion"])
server_debug_prog   = outter_env.Program('test_server',  ['test_server.cpp']  + objects["orion"])

env.Alias('orion', [client_debug_prog])
env.Alias('orion', [server_debug_prog])

env.Default(['orion'])
