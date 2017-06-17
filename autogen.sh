#! /bin/sh

( 
echo "autogen.sh was invoked by $LOGNAME@$HOSTNAME on `date`."
echo

set -x
# GNU Gettext
gettextize --copy --force --no-changelog
for f in Makefile.am configure.ac
do
  if test -f "$f"~
  then
    mv "$f"~ $f
  fi
done
(
  cd build-aux
  wget --no-verbose --timestamping \
    'http://savannah.gnu.org/cgi-bin/viewcvs/*checkout*/config/config/config.guess' \
    'http://savannah.gnu.org/cgi-bin/viewcvs/*checkout*/config/config/config.sub'
  chmod a+x config.guess config.sub
)

# GNU Portability Library
gnulib-tool --libtool --no-changelog --import \
  assert dirname error exit free getopt getline \
  malloc realloc size_max strtol

# GNU Autotools
autoreconf --install --force --include=build-aux --include=m4
) | 
tee autogen.log

echo
echo "The command output was saved in autogen.log."
