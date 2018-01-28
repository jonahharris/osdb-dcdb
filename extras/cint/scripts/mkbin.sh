if [ "$1" != "-o" ]; then
  echo "Usage: bash mkbin.sh -o binname cfilelist"
  echo "  where binname is the name of the resultant binary and"
  echo "  cfilelist is the list of C files to compile (may be just 1 C file)."
  return 1
fi

uname_s=`uname -s`
OS="Unknown"
if [ "$uname_s" == "Linux" ]; then 
  OS="Linux"
fi
if [ "$uname_s" == "SunOS" ]; then
  OS="Solaris"
fi
if [ "$OS" == "Unknown" ]; then
  uname_o=`uname -o`
  if [ "$uname_o" == "Cygwin" ]; then
    OS="Cygwin"
  fi
fi

if [ "$OS" == "Linux" ]; then
  OPTIMS="-g -march=i686 -mcpu=i686 -fomit-frame-pointer -fno-strength-reduce -fno-strict-aliasing -minline-all-stringops -m80387 -I../../../include -L../../../lib"
fi
if [ "$OS" == "Solaris" ]; then
  OPTIMS="-O3 -s -msupersparc -mcpu=supersparc -I../../../include -L../../../lib"
fi
if [ "$OS" == "Cygwin" ]; then
  OPTIMS="-O3 -s -march=i686 -mcpu=i686 -I../../../include -L../../../lib"
fi

gcc $OPTIMS $@ ../../cdb.c ../../container.c -lcdb -lcrypto -lssl
