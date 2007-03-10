#!/usr/bin/env python

# Written by Henry 'Pi' James and Loring Holden
# modified for multitracker display by John Hoffman
# see LICENSE.txt for license information

from sys import *
from os.path import *
from sha import *
from BitTornado.bencode import *
from types import FloatType
import re

def hum(bytes, allow_decimals = True):
	suf = ['', 'K', 'M', 'G'];
	idx = 0
	
	while idx < len(suf) - 1: # Check if we want another suffix
		if bytes % 1024 == 0:
			bytes = bytes / 1024
			idx += 1
		elif allow_decimals and bytes / 1024 >= 10:
			bytes = bytes / 1024.0
			idx += 1
		else:
			break
	
	
	if type(bytes) == FloatType:
		fmt = "%.1f%s"
	else:
		fmt = "%d%s"
	return fmt % (bytes, suf[idx])

print hum(22222)

NAME, EXT = splitext(basename(argv[0]))
VERSION = '20030621'

print '%s %s - decode BitTorrent metainfo files' % (NAME, VERSION)
print

if len(argv) == 1:
    print '%s file1.torrent file2.torrent file3.torrent ...' % argv[0]
    print
    exit(2) # common exit code for syntax error

for metainfo_name in argv[1:]:
    metainfo_file = open(metainfo_name, 'rb')
    metainfo = bdecode(metainfo_file.read())
#    print metainfo
    info = metainfo['info']
    info_hash = sha(bencode(info))

    print 'metainfo file.: %s' % basename(metainfo_name)
    print 'info hash.....: %s' % info_hash.hexdigest()
    piece_length = info['piece length']
    if info.has_key('length'):
        # let's assume we just have a file
        print 'file name.....: %s' % info['name']
        file_length = info['length']
        name ='file size.....:'
    else:
        # let's assume we have a directory structure
        print 'directory name: %s' % info['name']
        print 'files.........: '
        file_length = 0;
        for file in info['files']:
            path = ''
            for item in file['path']:
                if (path != ''):
                   path = path + "/"
                path = path + item
            print '   %s (%s)' % (path, hum(file['length']))
            file_length += file['length']
            name ='archive size..:'
    piece_number, last_piece_length = divmod(file_length, piece_length)
    print '%s %s (%i * %s + %s)' % (name, hum(file_length), piece_number,
    	hum(piece_length), hum(last_piece_length))
    print 'pieces........: %i' % (piece_number + 1)
    print 'piece size....: %s' % hum(piece_length, False)
    print 'announce url..: %s' % metainfo['announce']
    if metainfo.has_key('announce-list'):
        list = []
        for tier in metainfo['announce-list']:
            for tracker in tier:
                list+=[tracker,',']
            del list[-1]
            list+=['|']
        del list[-1]
        liststring = ''
        for i in list:
            liststring+=i
        print 'announce-list.: %s' % liststring
    if metainfo.has_key('httpseeds'):
        list = []
        for seed in metainfo['httpseeds']:
            list += [seed,'|']
        del list[-1]
        liststring = ''
        for i in list:
            liststring+=i
        print 'http seeds....: %s' % liststring
    if metainfo.has_key('comment'):
        print 'comment.......: %s' % metainfo['comment']
