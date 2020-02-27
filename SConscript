Import('env')

requires = ['websocketpp', 'json']

env.UnitTestDir('midgard_tests', 'tests',
                extra_src_dir='src', extra_inc_dir='include',
                requires=requires)
env.MainDir('.', requires=requires)
env.Install(env['bin_dir'].Dir('web-serve'), Glob('web-serve/*.*'))
