===== INTRO =====

Q. What is this?

A. This is a tiny toolset to let you download only part of a video via BitTorrent.


Q. Why would I want this?

A. I guess you've never wanted to see just the big play everybody is talking about, and not the whole game? Or just the part of a TV show you missed, and not the whole thing all over again? Maybe the movie scene where the actor/actress you have a crush on looks really hot, without all the arty fartsy "acting" bits?  

(Alternatively, maybe you only want to see the serious parts, without the silly pseudo-porn... but somehow I doubt it.)


Q. Why not just download the whole video, then watch only the parts you want?

A. The whole video could be huge, making it take a long time to download the whole thing. If you pay for your bandwidth, or have a bandwidth cap, that's another good reason not to grab it all.

(I suppose you could also do really weird things like downloading and watching the end of a movie before the rest of it, if suspense really bothers you.)


Q. Does it work?

A. Mostly. These tools are designed to work with video in AVI containers, so you have a pretty good chance if that's what you're trying to download. (It's not as user-friendly as I'd like, but I'm unlikely to fix that unless bribed.)

For obvious reasons, things won't work well with video wrapped in a RAR, or another similarly opaque format.


Q. Can I download just parts of other stuff too, like audio? How about software?

A. Sure, as long as the data you want is stored consecutively. Just ignore the instructions about extracting video, and do your own thing with the partially downloaded files. MP3 audio should work easily, and you could probably extract things from the middle of ISOs or tarballs if you really tried--don't ask me how though.


Q. What are the downsides? (AKA: What can you help fix?)

A.	- Finding the section of video that you want may take a bit of work.
	- Your audio and video may get out of sync.
	- Downloading just parts of a video via BitTorrent could be less efficient
		than downloading the whole thing, especially if you take a lot of
		tries to find the parts you want.


===== MISC =====

Q. Ok, so how do I get my video?

A. There are two parts, each has its own section below:
	1. Downloading just part of a torrent.
	2. Extracting your data from partially-downloaded (aka: "corrupt") files.


Q. What's this SVN thing you keep talking about?

A. It's my Subversion version control repository, at http://vasi.dyndns.org:3128/svn/avipart/trunk . If you like pretty things, there's a wonderful and pretty web interface at http://vasi.dyndns.org:3128/trac/browser/avipart/trunk .


Q. Oooh, shiny!

A. Indeed.


===== DOWNLOADING =====

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


Q. Help! How do I know what pieces to get?

A. Many formats (including AVI) keep important data in the header, so you almost always want to grab the first few pieces. Aside from that, just guess!

A reasonable strategy is to download a few small pieces in a first run, and use the results to decide what you really need. For example, suppose a file has 2000 pieces, and you think the part you want is around 1/3 of the way through. First use:

	--pieces 1-5,550-555,600-605,650-655,700-705,750-755,800-805

Now check what's in each of the segments you grabbed. If it turns out that 650-655 ad 750-755 both are in the part you want, then you can probably get it all with:

	--pieces 1-5,620-780

There's no harm in trying over and over, you don't have to re-download anything you've already gotten.


===== EXTRACTING =====

Q. That's all very nice, but how do I actually check what's in a segment of a file?

A. The depends what file it is. If it's an AVI, you're in luck, see the next section. Otherwise, you're on your own. You can use dd(1) to extract the segment that you downloaded, but that segment may or may not work on its own without further munging.


Q. Ooh, I feel so lucky! How do I extract a segment of an AVI?

A. First, compile avipart. Enter the 'extract' directory in SVN on the command line, and type 'make avipart'. If you're on Mac OS X, it should work!

Now run avipart, like so:

	./avipart input.avi output.avi PIECE-SIZE FIRST-PIECE

For example, if you just downloaded pieces 300-350 of StealThisMovie.avi, and btshowmetainfo.py told you each piece is 512k, then run:

	./avipart StealThisMovie.avi output.avi 512k 300

Now avipart will try to extract the section to output.avi, and if everything works well, you'll have a working movie!


Q. Hey, what's this 'if you're on Mac OS X' thing about?

A. There aren't any endian-swapping functions which are standard and cross-platform, so I used the ones on my platform.


Q. You should have written your own endian-swapping functions OR tested on many platforms OR at least have them modularized!!!

A. That's not a question, and I'll expect either a patch or a bribe.


Q. After I extract a movie, I can't jump forwards and backwards through it. What do I do?

A. Avipart has to get rid of a movie's index in order to extract arbitrary sections. But you can use mencoder to reconstruct the index. You can get mencoder from many different sources, it's part of mplayer. Once you have it installed, run:

	mencoder -forceidx extracted.avi -o fixed.avi -oac copy -ovc copy

After a couple of minutes, you'll have a movie with a working index.
