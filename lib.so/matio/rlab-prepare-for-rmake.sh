#!/bin/sh
rm libmatio.a
cd matio-master
make clean
cd ..

LIBA=./matio-master/src/.libs/libmatio.a

if ! [ -f ${LIBA} ]; then
  echo "Static MATIO library ${LIBA} does not exist: Building it"
  cd matio-master
  if ! [ -x ./configure ]; then
    echo -n "Generating 'configure' file for matio build"
    ./autogen.sh
    echo " Done"
  fi
  # this has to be done because configure build script sux
  make clean
  ./configure \
    --enable-static \
    --disable-shared \
    --enable-mat73=yes \
    --enable-extended-sparse=yes
  make all
  cd ..
  echo "Done!"
fi

if ! [ -f libmatio.a ]; then
  # put static matio library where rlab can build its shared library with it
  cp ${LIBA} ./
fi

if ! [ -f libmatio.a ]; then
  echo "Horrible internal error: Help! I need somebody! Help! Just anybody!"
  exit 1
fi

echo "We should be ready for 'rmake' now!"






 
