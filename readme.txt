== nQuake Readme ==

TOC:
- Introduction
- Hardware and Software requirements
- Installation
- Keyboard Controls
- How to get the Shareware PAK file
- Source Notes


== Introduction ==

nQuake is a port of WinQuake which sources were released by
Id Software under the GNU General Public License version 2.
It was ( and for some still is ) very popular because soon
after release one could create own content for the full
version like maps, new enemies and different game play
styles. It also featured mature multiplayer modes like
deathmatch where one was able to compete with other players
online for fame. Multiplayer is missing from this port.


== Hardware and Software requirements ==

You need a Nspire CX or Nspire CX CAS with Ndless 3.1 or
Ndless 3.6 installed. Monochrome models wont work.


== Installation ==

You need at least the PAK0.PAK from the shareware release.
If you bought Quake you can add PAK1.PAK and probably any
additional content to the game directory.

You need to create a directory structure like this on
your Nspire:

quake/nquake.tns		; the nquake executable
quake/id1/pak0.pak.tns	; the shareware episode pak file
quake/id1/pak1.pak.tns	; the full version pak file [optional]

id1/ is the game directory. Quake will store all game related
files like key re-mappings, video options and save games
there.

Once you have started Quake by running the nQuake executable
you will hopefully be presented by a demo playback. If Quake
complains about not finding gfx.wad the pak0.pak was not
found at the expected location.

In case you cannot put the pak file(s) in a id1 subdirectory
relative to the nquake executable because you cannot put files
in subdirectories with you link program AND you cannot upload
the pak files to the calculator and then "Cut/Paste" or
"Save As" them to an id1 directory you can create a file called
nquake.cmd.tns in the directory where the nQuake executable
is located. You can specify a commandline in this file which
is parsed by nQuake upon startup. With the commandline parameter
'-game <dir>' you can set an additional directory in which
nQuake will look for pak files. So if you put your pak files
in the same directory as the nQuake executable this file
should contain '-game ""' ( without the ' ). An example of
such a file is provided.


== Keyboard Controls ==

The escape key brings up the in-game menu. There you can
navigate using the arrow keys and select menu items with
Enter. The default key binding is sub-optimal and it is
recommended to re-map the keys using the Options menu item
customize controls.
nQuake allows the mapping of the following keys:
Arrow Keys, A-Z, 0-9, SPACE, TAB, CTRL, SHIFT, SPACE,+/-/=,
TRIG, ^, X^2, e^x, 10^x, (), .

This enables one to steer with the right hand on the
Touchpad and have the left hand free to press, for example,
CTRL to shoot, = and TRIG to look up and down, ^ to jump,
e^x and 10^x to swim up and down etc.

The Quake console is bound by default to the ?! key.


== How to get the shareware PAK file ==

Get the quake shareware release quake106.zip somewhere of
the internet. Unzip this, inside is a file called resource.1.
Rename resource.1 to resource.x. Use the LHa compression/
uncompression utility, which you can get from the internet
too, to unpack the resource.x like so: "lha x resource.x".
This will decompress the whole shareware archive which will
create a folder called id1 with the pak0.pak inside it. I
suggest to do this in a temporary directory so excessive
files do not polute your file system
To find quake106.zip and a build of LHa I suggest Google for
Windows or, for Linux, the packet manager of your choice.


== Source Notes ==

The system specific stuff is in vid_nspire.c sys_nspire.c
and sys_nspires.S. The Video driver is unspectacular, it
just sets the PL111 controller to palette mode and copies
the screen buffer to it upon request. The System functions
are somewhat more interesting as IO appears to be slow on
the Nspire so the routine that reads the keyboard state is
of some value. The assembler functions are used to either
align the stack to 8 byte as the ARM EABI requires ( which
Ndless does not seem to do ) so functions that take 8 byte
datatypes work ( printf with doubles and the like ) or
redirect the stack to some user allocated area as it seems
the normal stack is just some 56k deep ( and Quake
requires > 250k ).

More general optimizations are scattered all around the
place. Most are marked with #defines and are float to
fixed point conversations. The rasterizer optimizations
for wall spans and alias models are in d_scan_nspirec.c
and d_scan_nspire.S. The way I implemented perspective
texture mapping of the wall spans originates from what
I did in Delsgolf for the TI-89. To see what I changed
in detail I suggest getting the WinQuake sources as
released by Id Software and do a diff.

If you want to build the Windows version of nQuake you
need to get the libraries of the original release as I
have not included them because the license of these is
unclear. You can run the Windows version in 800x600
or higher to check for rendering artifacts caused by
the fixed point stuff.

--
Ralf Willenbacher ( ralf.willenbacher@live.com )
