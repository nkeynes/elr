TESTS = testskel

check_PROGRAMS = testskel

BUILT_SOURCES = config.c

testskel_SOURCES = config.c testdrv.c

config.c: config.elr ../src/elr $(srcdir)/../src/skel/skel.c
	../src/elr config.elr -o config.c -S $(srcdir)/../src/skel
