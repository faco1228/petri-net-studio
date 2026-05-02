# Makefile — top level
# Autori: xfacka00, xlogin02

LOGIN1 = xfacka00
LOGIN2 = xlogin02
ARCHIVE = $(LOGIN1)-$(LOGIN2).zip

.PHONY: all run test doxygen clean pack

all:
	$(MAKE) -C src all

run: all
	open ./src/icp_petri.app

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
	    $$(pkg-config --cflags --libs Qt6Core 2>/dev/null || echo "-I/opt/homebrew/lib/QtCore.framework/Headers -F/opt/homebrew/lib -framework QtCore") \
	    -o test_roundtrip && ./test_roundtrip

doxygen:
	doxygen Doxyfile

clean:
	$(MAKE) -C src clean
	rm -rf doc/html doc/latex
	rm -rf generated/*.cpp generated/interpreter_*
	rm -f $(ARCHIVE)

pack: clean
	zip -r $(ARCHIVE) \
	    Makefile \
	    README.txt \
	    Doxyfile \
	    src/ \
	    examples/ \
	    design.pdf
	@echo "Archív vytvorený: $(ARCHIVE)"
	@echo "Veľkosť: $$(du -sh $(ARCHIVE))"
