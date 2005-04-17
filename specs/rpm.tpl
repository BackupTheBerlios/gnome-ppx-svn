Name: @name@
Summary: @summary@
Version: @version@
Release: @release@
BuildArchitectures: @arch@
Source: @source@
Vendor: @maintainer@
URL: @url@
License: @license@
Group: @group@
Provides: @name@ = @version@
Buildroot: /var/tmp/%{name}-%{version}-%{release}-root
Prefix: %{_prefix}

%description
@description@

%prep

%setup

%build
<?
print_sect (prepare, '%configure ')
print_sect (compile, '%{__make} ')
?>

%install
%{__rm} -rf %{buildroot}
<?
print_sect (install, "%makeinstall ")
?>

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root)
<?
for d in files:
	if d == 'sysconf':
		print_files ('%config(noreplace) %{_sysconfdir}/' , "", files[d])
	else:
		print_files ("%{_" + d + "dir}/", "", files[d])

?>
