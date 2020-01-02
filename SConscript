Import('env')

env.MainDir('.', requires=['websocketpp','json'])
env.Install(env['bin_dir'].Dir('web-serve'), Glob('web-serve/*'))
