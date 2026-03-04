SQPROF_CXXFLAGS = -pg -pie -g3
SQ64_CXXFLAGS = -m64 -D_SQ64

all: sq32

sq32: folders
	$(MAKE) -C squirrel
	$(MAKE) -C sqstdlib
	$(MAKE) -C sq

sqprof: folders
	CXXFLAGS="$(SQPROF_CXXFLAGS)" $(MAKE) -C squirrel
	CXXFLAGS="$(SQPROF_CXXFLAGS)" $(MAKE) -C sqstdlib
	CXXFLAGS="$(SQPROF_CXXFLAGS)" $(MAKE) -C sq

sq64: folders
	CXXFLAGS="$(SQ64_CXXFLAGS)" $(MAKE) -C squirrel
	CXXFLAGS="$(SQ64_CXXFLAGS)" $(MAKE) -C sqstdlib
	CXXFLAGS="$(SQ64_CXXFLAGS)" $(MAKE) -C sq

folders:
	mkdir -p lib bin

clean:
	$(MAKE) -C squirrel clean
	$(MAKE) -C sqstdlib clean
	$(MAKE) -C sq clean