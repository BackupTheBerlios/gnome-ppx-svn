#!/bin/bash
set -x
cd $(dirname $0)
tarball="@name@-@version@.tar.gz"
MAKEPKG="$PWD/specs/makepkg"
origindir="$PWD"
tmpdir="/tmp"
destdir="/tmp/dest-@name@-@version@"
# Generate tarball
[ ! -f "$tarball" ] && make dist
if [ ! -f "$tarball" ]; then
	echo "Error, source code not found: $tarball"
	exit 1
fi

rm -rf "$destdir"
rm -rf "$tmpdir/@name@-@version@"
mkdir -p "$tmpdir"
mkdir -p "$destdir"
tar -C "$tmpdir" -zxf "$tarball"
cd "$tmpdir/@name@-@version@"
<?
print_sect (prepare, "./configure ", " --prefix=/usr --sysconfdir=/etc")
print_sect (compile, "make ")
print_sect (install, "make install ", " DESTDIR=\"$destdir\"")

?>[ ! -d "$destdir"/install ] && mkdir -p "$destdir/install"
cat << EOF > $destdir/install/slack-desc
@name@: @name@ @version@ @summary@
@name@:
<?
desc = description.replace("\n"," ").replace("  "," ")
lines = 0
lines_len = 60

while len(desc) and lines < 8:
	print name + ": " + desc[:lines_len]
	desc = desc[lines_len:]
	lines += 1

while lines < 8:
	print name + ":"
	lines += 1
?>@name@: Package Created By: @maintainer@
EOF
strip "$destdir"/usr/{sbin,bin,libexec}/*
cd "$destdir"
# Fix permissions
chown -R root.root .
find . -perm 777 -exec chmod 755 {} \;
find . -perm 555 -exec chmod 755 {} \;
find . -perm 444 -exec chmod 644 {} \;
find . -perm 666 -exec chmod 644 {} \;
find . -perm 664 -exec chmod 644 {} \;
"$MAKEPKG" "$origindir"/@name@-@version@-@arch@-@release@.tgz
