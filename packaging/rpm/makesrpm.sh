#!/bin/bash
#-------------------------------------------------------------------------------
# Create a source RPM package - adopted from xrootd
# Author: Lukasz Janyst <ljanyst@cern.ch> (16.10.2014)
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Find a program
#-------------------------------------------------------------------------------
function findProg()
{
  for prog in $@; do
    if test -x "`which $prog 2>/dev/null`"; then
      echo $prog
      break
    fi
  done
}

#-------------------------------------------------------------------------------
# Print help
#-------------------------------------------------------------------------------
function printHelp()
{
  echo "Usage:"                                              1>&2
  echo "${0} [--help] [--source PATH] [--output PATH]"       1>&2
  echo "  --help        prints this message"                 1>&2
  echo "  --source PATH specify the root of the source tree" 1>&2
  echo "                defaults to ../"                     1>&2
  echo "  --output PATH the directory where the source rpm"  1>&2
  echo "                should be stored, defaulting to ."   1>&2
}

#-------------------------------------------------------------------------------
# Parse the commandline, if only we could use getopt... :(
#-------------------------------------------------------------------------------
SOURCEPATH="../../"
OUTPUTPATH="."
PRINTHELP=0

while test ${#} -ne 0; do
  if test x${1} = x--help; then
    PRINTHELP=1
  elif test x${1} = x--source; then
    if test ${#} -lt 2; then
      echo "--source parameter needs an argument" 1>&2
      exit 1
    fi
    SOURCEPATH=${2}
    shift
  elif test x${1} = x--output; then
    if test ${#} -lt 2; then
      echo "--output parameter needs an argument" 1>&2
      exit 1
    fi
    OUTPUTPATH=${2}
    shift
  fi
  shift
done

if test $PRINTHELP -eq 1; then
  printHelp
  exit 0
fi

echo "[i] Working on: $SOURCEPATH"
echo "[i] Storing the output to: $OUTPUTPATH"

#-------------------------------------------------------------------------------
# Check if the source and the output dirs
#-------------------------------------------------------------------------------
if test ! -d $SOURCEPATH -o ! -r $SOURCEPATH; then
  echo "[!] Source path does not exist or is not readable" 1>&2
  exit 2
fi

if test ! -d $OUTPUTPATH -o ! -w $OUTPUTPATH; then
  echo "[!] Output path does not exist or is not writeable" 1>&2
  exit 2
fi

#-------------------------------------------------------------------------------
# Check if we have all the necassary components
#-------------------------------------------------------------------------------
if test x`findProg rpmbuild` = x; then
  echo "[!] Unable to find rpmbuild, aborting..." 1>&2
  exit 1
fi

if test x`findProg git` = x; then
  echo "[!] Unable to find git, aborting..." 1>&2
  exit 1
fi

#-------------------------------------------------------------------------------
# Check if the source is a git repository
#-------------------------------------------------------------------------------
if test ! -d $SOURCEPATH/.git; then
  echo "[!] I can only work with a git repository" 1>&2
  exit 2
fi

#-------------------------------------------------------------------------------
# Create a tempdir and copy the files there
#-------------------------------------------------------------------------------
# exit on any error
set -e

TEMPDIR=`mktemp -d /tmp/cppunit-text-runner.srpm.XXXXXXXXXX`
RPMSOURCES=$TEMPDIR/rpmbuild/SOURCES
mkdir -p $RPMSOURCES
mkdir -p $TEMPDIR/rpmbuild/SRPMS

echo "[i] Working in: $TEMPDIR" 1>&2

#-------------------------------------------------------------------------------
# Generate the spec file
#-------------------------------------------------------------------------------
if test ! -r cppunit-text-runner.spec; then
  echo "[!] The specfile does not exist!" 1>&2
  exit 7
fi
cp cppunit-text-runner.spec $TEMPDIR

#-------------------------------------------------------------------------------
# Make a tarball of the latest commit on the branch
#-------------------------------------------------------------------------------
# no more exiting on error
set +e

CWD=$PWD
cd $SOURCEPATH
COMMIT=`git log --pretty=format:"%H" -1`

if test $? -ne 0; then
  echo "[!] Unable to figure out the git commit hash" 1>&2
  exit 5
fi

git archive --prefix=cppunit-text-runner/ --format=tar $COMMIT | gzip -9fn > \
      $RPMSOURCES/cppunit-text-runner.tar.gz

if test $? -ne 0; then
  echo "[!] Unable to create the source tarball" 1>&2
  exit 6
fi

#-------------------------------------------------------------------------------
# Build the source RPM
#-------------------------------------------------------------------------------
echo "[i] Creating the source RPM..."

# Dirty, dirty hack!
echo "%_sourcedir $RPMSOURCES" >> $TEMPDIR/rpmmacros
rpmbuild --define "_topdir $TEMPDIR/rpmbuild"    \
         --define "%_sourcedir $RPMSOURCES"      \
         --define "%_srcrpmdir %{_topdir}/SRPMS" \
         --define "_source_filedigest_algorithm md5" \
         --define "_binary_filedigest_algorithm md5" \
  -bs $TEMPDIR/cppunit-text-runner.spec > $TEMPDIR/log
if test $? -ne 0; then
  echo "[!] RPM creation failed" 1>&2
  exit 8
fi

cd $CWD
cp $TEMPDIR/rpmbuild/SRPMS/*.src.rpm $OUTPUTPATH
rm -rf $TEMPDIR

echo "[i] Done."
