BIN=${CZTNOISECLEAN}/bin/
SRC=${CZTNOISECLEAN}/src/
CFITSIO=-lcfitsio
compile_gcc=-I${as1czt}/include/ -L${as1czt}/lib/ ${CFITSIO}  -I${as1czt}/lib/pil  -lpil -lm

all:
	gcc ${SRC}cztdataselection.c $(compile_gcc) -o ${BIN}cztdataselection
	gcc ${SRC}cztnoisypixclean.c $(compile_gcc) -o ${BIN}cztnoisypixclean 
	gcc ${SRC}cztsuperbunchclean.c $(compile_gcc) -o ${BIN}cztsuperbunchclean 
	gcc ${SRC}cztheavybunchclean.c $(compile_gcc) -o ${BIN}cztheavybunchclean 
	gcc ${SRC}cztflickpixclean.c $(compile_gcc) -o ${BIN}cztflickpixclean
	gcc ${SRC}czteventsep.c $(compile_gcc) -o ${BIN}czteventsep
