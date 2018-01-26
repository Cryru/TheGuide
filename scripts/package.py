#!python

import zipfile
import tempfile
import os
import sys
import shutil
import ftplib

TAG = 'REL_2.0'
VERSION = '2.0'

def build():
	print 'building binaries:'
	if 'MSVCDir' not in os.environ:
		print '  ** please do a vsvars32.bat first.'
		sys.exit(1)
	cmd = 'devenv ..\\winguide.sln /rebuild Release'
	print '  invoking devenv'
	r = os.system(cmd)
	if r != 0:
		print '  ** devenv exited with non-zero exit code(%d)' % r
		sys.exit(2)

def package():
	print 'packaging binaries:'
	cwd = os.getcwd()
	os.chdir('../setup')
	cmd = 'makensis /V1 guide-setup.nsi'
	print '  invoking makensis'
	r = os.system(cmd)
	os.chdir(cwd)
	if r != 0:
		print '  ** makensis exited with non-zero exit code(%d)' % r
		sys.exit(3)
	shutil.copy('../setup/winguide-%s-win32.exe' % VERSION, '.')

def create_zip():
	print 'packaging binary zip:'
	zf = zipfile.ZipFile("winguide-%s-win32.zip" % VERSION, 'w', zipfile.ZIP_DEFLATED)
	zf.write('../CHANGES', 'CHANGES')
	zf.write('../guide/Release/Guide.exe', 'Guide.exe')
	zf.write('../gdeutil/Release/gdeutil.exe', 'gdeutil.exe')
	zf.write('../guide/Guide.exe.manifest', 'Guide.exe.manifest')
	zf.write('../guide/guide.ini', 'guide.ini')
	zf.write('../libguide/Release/libguide.dll', 'libguide.dll')
	zf.write('../LICENSE', 'LICENSE')
	zf.write('../README', 'README')
	zf.write('../NOTICE', 'NOTICE')
	zf.write('../setup/mfc71u.dll', 'mfc71u.dll')
	zf.write('../setup/msvcr71.dll', 'msvcr71.dll')
	zf.write('../setup/msftedit.dll', 'msftedit.dll')
	zf.close()

def _rmdir_all(top):
	for root, dirs, files in os.walk(top, topdown=False):
		for name in files:
			os.remove(os.path.join(root, name))
		for name in dirs:
			os.rmdir(os.path.join(root, name))
	os.rmdir(top)

def create_src():
	print 'packaging source zip:'
	tempdir = tempfile.mkdtemp()
	expsrcdir = os.path.join(tempdir, 'src')
	cmd = 'svn export --quiet file:///c:/svnrepos/winguide/tags/%s "%s"' % (TAG, expsrcdir)
	if os.system(cmd) == 0:
		zf = zipfile.ZipFile("winguide-%s-src.zip" % VERSION, 'w', zipfile.ZIP_DEFLATED)
		for root, dirs, files in os.walk(expsrcdir):
			relpath = root.replace(expsrcdir, "")
			if len(relpath) and relpath[0] == '\\': relpath = relpath[1:]
			for name in files:
				zipname = os.path.join(relpath, name)
				fname   = os.path.join(root, name)
				zf.write(fname, zipname)
		zf.close()
	_rmdir_all(tempdir)

def _stor(ftp, f):
	print '  stor-ing', f
	ftp.storbinary('STOR ' + f, file(f))

def upload_all():
	print 'uploading files to sourceforge'
	print '  connecting'
	ftp = ftplib.FTP('upload.sourceforge.net', 'anonymous', 'mdevan@users.sf.net')
	print ftp.getwelcome()
	ftp.cwd('/incoming')
	_stor(ftp, 'winguide-%s-src.zip' % VERSION)
	_stor(ftp, 'winguide-%s-win32.zip' % VERSION)
	_stor(ftp, 'winguide-%s-win32.exe' % VERSION)
	print '  quiting'
	ftp.quit()
	ftp.close()


build()
package()
create_zip()
create_src()
#upload_all()
