Q. How do I download part of a torrent?

A. Use my custom version of BitTornado. Either use the modified snapshot of 0.3.18 in my SVN, or apply BitTornado.patch from my SVN to a newer version of BitTornado.

Then from the command-line, run './btshowmetainfo.py mytorrent.torrent'. The output should include something like this:

	pieces........: 2794
	piece size....: 256K

Each torrent is divided into a bunch of pieces. This tells you how many pieces there are (here: 2794) and how large each piece is (here: 256 kibibytes).

Now you can run './btdownloadcurses.py mytorrent.torrent --pieces 2000-2500' to grab only pieces 2000 thru 2499 of the torrent. You can supply more complicated piece specifications, eg: '--pieces 1-5,30,300-500,9000-' would download pieces 1 thru 4, piece 30, pieces 300 thru 499, and 9000 thru the end of the torrent.

Note that piece specifications are one-based, ie: piece 1 is the first piece, not piece 0.


Q. You dolt! Don't you know that computers number things from zero?

A. I arbitrarily decided to number pieces from one when I started, and now I'm too lazy to change it. Fix it yourself!
