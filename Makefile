# Makefile — top level
# Autori: xfackas00, xhanzea00

LOGIN1 = xfackas00
LOGIN2 = xhanzea00
ARCHIVE = $(LOGIN1)-$(LOGIN2).zip

# Detect qmake binary (qmake6 on newer distros, qmake on older / macOS)
QMAKE = $(shell which qmake6 2>/dev/null || which qmake 2>/dev/null || echo qmake)

.PHONY: all run test doxygen clean pack

all:
	cd src && $(QMAKE) icp_project.pro && $(MAKE)

run: all
	./src/icp_petri.app/Contents/MacOS/icp_petri 2>/dev/null || ./src/icp_petri

test:
	cd tests && g++ -std=c++17 \
	    -I../src/inc -I../src/model \
	    test_roundtrip.cpp \
	    ../src/model/pn_net.cpp \
	    ../src/model/pn_place.cpp \
	    ../src/model/pn_transition.cpp \
	    ../src/model/pn_arc.cpp \
	    ../src/model/pn_file_parser.cpp \
	    ../src/model/pn_file_writer.cpp \
	    $$(pkg-config --cflags --libs Qt6Core 2>/dev/null || \
	       pkg-config --cflags --libs Qt5Core 2>/dev/null || \
	       echo "") \
	    -o test_roundtrip && ./test_roundtrip

doxygen:
	doxygen Doxyfile

clean:
	-$(MAKE) -C src clean 2>/dev/null || true
	rm -rf src/Makefile src/*.o
	rm -rf src/icp_petri src/icp_petri.app
	rm -rf src/.qmake.stash src/moc_predefs.h
	rm -rf src/moc_*.cpp src/moc_*.o
	rm -rf doc/html doc/latex
	rm -rf generated/*.cpp generated/interpreter_*
	rm -f tests/test_roundtrip
	rm -f $(ARCHIVE)

pack: clean
	zip -r $(ARCHIVE) \
	    Makefile \
	    README.txt \
	    Doxyfile \
	    src/ \
	    doc/ \
	    examples/ \
	    tests/
	@echo "Archív vytvorený: $(ARCHIVE)"
	@echo "Veľkosť: $$(du -sh $(ARCHIVE))"
