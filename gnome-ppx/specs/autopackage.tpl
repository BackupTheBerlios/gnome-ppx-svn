# -*-shell-script-*-
[Meta]
RootName: @namespace@:@version@
DisplayName: @name_full@
ShortName: @name@
Maintainer: @maintainer@
Packager: @packager@
Summary: @summary@
SoftwareVersion: @version@
PackageVersion: @release@
@header@

[Description]
@description@

[BuildPrepare]
<?print_sect(prepare, "prepareBuild ")?>

[BuildUnprepare]
unprepareBuild

[Imports]
echo '*' | import

[Prepare]
<?
for dep in dependencies:
	print "require " + dep
?>

[Install]
<?
for d in files:
	if d == 'bin' or d == 'sbin':
		print_files ("installExe " + dirname[d] + "/", "", files[d])
	else:
		print_files ("copyFiles " + dirname[d] + "/", " $prefix/" + dirname[d], files[d])
?>
@install_extra@

[Uninstall]
uninstallFromLog
