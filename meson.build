project('merge', 'c', version: 'prealpha')

	#version: run_command('python', 'get_version.py', check: true).stdout().strip())
cc = meson.get_compiler('c')
src = ['src/main.c', 'src/loop.c']
deps = [dependency('libevent')]
#cc.find_library('m', required : false)]
add_global_arguments('-g', language : 'c')
#add_global_arguments('-O2', language : 'c')
add_global_arguments('-Wall', language : 'c')
add_global_arguments('-Wextra', language : 'c')
add_global_arguments('-Wshadow', language : 'c')
add_global_arguments('-pedantic', language : 'c')
add_global_arguments('-Wno-unused-parameter', language : 'c')
add_global_arguments('-Wno-unused-function', language : 'c')
executable('merge', src, dependencies: deps, c_args: '-DVERSION="@0@"'.format(meson.project_version()), install: true)
