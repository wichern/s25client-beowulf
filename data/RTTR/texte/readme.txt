                              RETURN TO THE ROOTS
--------------------------------------------------------------------------------

A. Reference Note
B. Installation
  1. Windows
  2. Linux
  3. Mac OSX
  4. Settings directory
C. Game
  1. Create a game
  2. Multiplayer game
  3. Replays
  4. Options
D. Crash and bugs
E. Summary: Updates and changelog
F. Licenses

--------------------------------------------------------------------------------

A. Reference Note

  The game requires an OpenGL2.0-compatible graphics card with at
  least 64 MB graphics-memory. A CPU with 800Mhz will suffice.

  Further on you will need an installed version of "The Settlers 2
  Gold-Edition" or the original version + Mission-CD.

--------------------------------------------------------------------------------

B. Installation

  1. Windows

    Windows 7 and 10 are supported. Windows Vista or older might work but
    are not officially supported.

    To play the game search for the "DATA" und "GFX" directories in the
    original S2 Gold (or S2 + Mission CD) game (from a valid installation
    or directly from the CD) and copy them into the nightly folder
    (where you find the file "Put your S2-Files in here").

    Alternatively you can make NTFS symbolic links as Administrator:
    mklink /D DATA "C:\S2\DATA"
    mklink /D GFX "C:\S2\GFX"

    To play RTTR, you simply have to run s25client.exe.

    (In Nightly's you can update with RTTR.BAT)

    Note: RTTR does not change any files from the original game.
    It acts as a game-mod.

   -----------------------------------------------------------------------------

  2. Linux

    You will need the following packages:
    (Use your package-manager or do it manually)

    libsdl2 libsdl-mixer2 gettext

    Make sure that DirectRendering works:

    glxinfo | grep direct

    When the output "direct rendering: Yes" appears, you are ok.
    If not, your performance will be lousy. Please check your
    graphics acceleration.

    Installation:

    extract RTTR archive e.g. to /opt/s25rttr

    mkdir -p /opt/s25rttr
    cd /opt/s25rttr
    tar -jxvf s25rttr_*.tar.bz2

    Now all you have to do is copying your original files to:
    /opt/s25rttr/share/s25rttr/S2

    Alternatively you can make a symbolic link to your original folder.

    /opt/s25rttr/bin/rttr.sh will launch the game.

    (Nightly versions automatically update when you start rttr.sh.
     Use "sh rttr.sh noupdate" to keep the actual version.)

  ------------------------------------------------------------------------------

  3. Mac OSX

    Start the game directly from the downloaded app bundle.

  ------------------------------------------------------------------------------

  4. Settings directory

    The settings directory is found in:

    Windows: <UserDir>/My Games/Return To The Roots
    Linux:   ~/.s25rttr
    Mac OSX: ~/Library/Application Support/Return To The Roots

    This folder is created on first start if it does not exist.
    All logs, settings, saves and replays are in subfolders to that.

--------------------------------------------------------------------------------

C. Game

  1. Create a game

    There is a Singleplayer- and a Multiplayer-Modus which can be found in
    the main menu. You can also play alone in the Multiplayer-Modus:

    1. Choose "Multiplayer"
    2. "Direct IP"
    3. "Create game"
    4. In the next window choose the name of the game, no password needed.
    5. In the map screen you can choose from one of the categories on the
       left side (similar to the original) and then a map or you can load
       a game ("Load Game" button)
    6. After choosing your map, click on "Continue" which leads you
       to the host-menu.
       The top part of this menu shows you the players and their
       configuration.
       Notice: To start the game you have to fill all player slots with
       computer players or human players.
       Alternatively you can close the slots by clicking on them.

    For a quick Singleplayer-Game click "Unlimited Play" in the
    Singleplayer-Menu and follow the steps 5 & 6 above.

  2. Multiplayer game

    a) Direct game
       The proceeding is equivalent to point 1. Other human players
       have to choose "Direct IP", "Join game" and then enter the IP or
       hostname. "Connect" will lead them into the host menu.

    b) Lobby access
       The lobby is used for creating your own games or for joining
       games of other players. A chat function is also integrated.

       You need to create a forum account on http://www.siedler25.org
       and use that to login into the game lobby.

    c) LAN games
       The LAN area is similar to the lobby but shows only maps created
       in the current LAN and does not require a login to join. You can
       also use Hamachi or a similar program to create virtual LANs over
       the internet.

    RTTR uses port 3665 (TCP). This port must be open to create a
    game and makes other people able to join your game. If you use a
    router, you have to  forward this port to your workstation.
    Look for "Virtual server" or "Port forwarding" in your router's
    menu. If you use a personal firewall on your workstation, TCP port
    3665 has to be allowed there too. This method works on internet
    and LAN.
    In LAN mode ports 3666 and 3667 (UDP) are used to find games.

    It's not necessary to have the game's map on every machine. It
    will automatically be transfered from the host to every player.

    A coloured snail symbol in the top right of the display indicates
    a bad connection to the player with same colour.

    The game will be stopped when the host leaves and cannot be
    continued.

    To pause the game, press "P" (Host only) and again to unpause.
    To chat with other players press "Enter".

  3. Replays

    Replays record every action in a game. You can watch them by
    choosing "Single player" - "Play replay".

    The keys [+] and [-] will raise or lower the replay-speed. "J" will make
    you able to jump to a specific gameframe.

    All replays can be found in the subfolder Replays in the game settings dir.

  4. Save

    Saving a game works just like in the original game. It also works at any
    position in a replay.
    To load a multiplayer savegame choose "Load game" in the map menu. 
    For singleplayer there is a button in the singleplayer menu.
    It is sufficient if only one player has the savegame file.


  5. Options

    The options in the main menu are self-explanatory.

--------------------------------------------------------------------------------

D. Crash and bugs

  There may still be crashes, asynchronities, save-bugs and other bugs
  in the game. We are currently working on this very hard.

  We can reproduce bugs better if there is a replay available. You
  can help us by sending your replays (subfolder Replays in the settings 
  folder) with a describtion of the bug. Be sure you used the latest 
  nightly-build version.

  If there is an async-bug you will find additional logs in the folder "Logs"
  in the settings folder. Please send us the logs of EVERY player and all
  of the replays accordingly.
  It is important that you tell us which version of RTTR you used.
  For example: "20170630-49c9bb8(nightly) or 0.8.2"

  Also make sure you did not make the async on purpose by cheating
  or using different versions.

  If you find bugs, please report them on:

      https://github.com/Return-To-The-Roots/s25client

  By using Github we can keep the bug reports organized. Also it makes it
  easier for you to see if your bug is already fixed. It also allows further
  communication between you and us, e.g. when we need more information
  from you.

  Alternatively you can post the bug in the forum or visit us on Discord:
  https://discord.gg/kyTQsSx
  You can also join the IRC-channel and Discord by visiting our homepage.

  Thanks a lot

  Settlers Freaks
  July 7th 2017

--------------------------------------------------------------------------------

E. Summary: Updates and changelog

  * 0.9.4 - 06.01.2022
  ------------------------------------------------------------------------------
  - Various fixes for bugs leading to unloadable savegames and crashes
  - Fix drawing issues related to high terrain
  - Pressing ESC now does no longer discard pending changes of setting windows
  - Fix uncloseable action window
  - On game start reopen windows opened in last game and restorr their positions
  - Fix faulty version handling (visual issues and unable to join other players)

  * 0.9.1 - 24.07.2021
  ------------------------------------------------------------------------------
  - Fullscreen mode on all drivers and OSs
  - Random map generator
  - Allow special chars in user name
  - Fix some bugs, crashes and asyncs
  - Map editor included
  - Improved performance

  * 0.8.2 - 22.08.2017
  ------------------------------------------------------------------------------
  - Many async fixes
  - Usage of UTF8 to support more languages
  - Many addons added
  - Support for all S2 terrain types
  - Zoom function
  - Lua scripting
  - LAN Lobby
  - Code quality improvements

  * 0.8.1
  ------------------------------------------------------------------------------
  - Bug fixes
  - AI Improvements
  - Complete Seafaring

  * 0.8.0
  ------------------------------------------------------------------------------
  - Greatly improved AI player
  - A lot of bugs fixed
  - Speed improvements
  - Observation windows

 * 0.7.2 - 17.01.2011 *
  ------------------------------------------------------------------------------
  - Critical Bugfixes

  * 0.7 - Seventh version - 24.12.2010 *
  ------------------------------------------------------------------------------
  - OpenSource!
  - Translation: Dutch
  - Translation: Russian (Font is missing)
  - Translation: Czech
  - Translation: Estonian
  - Translation: Italian
  - Translation: Norwegian
  - Translation: Polish
  - Translation: Slovenian
  - Translation: Slovak
  - Statistics
  - Postoffice
  - Diplomacy - unfinished
  - First AI (jh)
  - Seafaring (not finished yet)
  - Addon Menu

  * 0.6 - Sixth version - 25.01.2009 *
  ------------------------------------------------------------------------------
  - Fixed a lot of bugs
  - Fog of War (with Teamview-option)
  - Watchout tower added
  - Minimap
  - Minimap in hostgame-menu
  - Planer
  - Balancing
  - Helper animations added
  - Multilanguage-support
  - Translation: Spanish
  - Translation: Hungarian
  - Translation: Swedish
  - Translation: Finnish (not completed)
  - Translation: French (not completed)
  - Victory-messages
  - HotKeys + Readme
  - Hostmenu option: Demolition prohibition
  - Building Info
  - Some building animations added

  * 0.5 - Fifth version - 27.01.2008 *
  ------------------------------------------------------------------------------
  - Fixed a lot of bugs
  - Donkeyraods, Donkey breeder
  - Boatroads with boats and shipyard (shipyard only builds boats)
  - Mountain road
  - Catapults
  - Soldier-behaviour changed (queue)
  - Burn down the depot makes all people escape somewhere
  - Async-log
  - Changed some action windows and tooltips
  - Sound on/off in the game

  * 0.4 - Fourth version - 09.10.2007 *
  ------------------------------------------------------------------------------
  - Save and load games, Autosave
  - Fixed very bad bugs

  * 0.3 - Third version Fix01 -  13.09.2007 *
  ------------------------------------------------------------------------------
  - critical crash-bug fixed

  * 0.3 - Third version - 12.09.2007 *
  ------------------------------------------------------------------------------
  - fixed crash-bugs
  - Settlers are in queue when the place in front is occupied
  - Settings-file in ~/.s25rttr (Linux)
  - New format for settings (GER-File)
  - Build-System is now cmake (Linux)
  - CIA-Bot for irc-channel
  - Lobby
  - Preperation for load and save

  * 0.2 - Second version - 15.07.2007 *
  ------------------------------------------------------------------------------
  - Fixed a lot of bugs
  - New screen resolutions
  - Kicking asynchron players now
  - Inventory-window
  - "Take out/stop storage" works now
  - "Ready"-Button in host-menu
  - Distribution of goods works
  - Adornment objects added (Ruins,...)
  - Jump to every house works now
  - Some adornment object are demolished when setting a road
  - Status of houses now visible (C, S)
  - Demolish-interrogation
  - RoadWindow closes now, when clicking somewhere else

  * 0.1 - First Release - 01.07.2007 *
  ------------------------------------------------------------------------------
  - Everything! ;-)

--------------------------------------------------------------------------------

F. Licenses

  1. S2.5 RTTR

                        GNU GENERAL PUBLIC LICENSE
                         Version 2, June 1991
  
   Copyright (C) 1989, 1991 Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
   Everyone is permitted to copy and distribute verbatim copies
   of this license document, but changing it is not allowed.
  
                              Preamble
  
    The licenses for most software are designed to take away your
  freedom to share and change it.  By contrast, the GNU General Public
  License is intended to guarantee your freedom to share and change free
  software--to make sure the software is free for all its users.  This
  General Public License applies to most of the Free Software
  Foundation's software and to any other program whose authors commit to
  using it.  (Some other Free Software Foundation software is covered by
  the GNU Lesser General Public License instead.)  You can apply it to
  your programs, too.
  
    When we speak of free software, we are referring to freedom, not
  price.  Our General Public Licenses are designed to make sure that you
  have the freedom to distribute copies of free software (and charge for
  this service if you wish), that you receive source code or can get it
  if you want it, that you can change the software or use pieces of it
  in new free programs; and that you know you can do these things.
  
    To protect your rights, we need to make restrictions that forbid
  anyone to deny you these rights or to ask you to surrender the rights.
  These restrictions translate to certain responsibilities for you if you
  distribute copies of the software, or if you modify it.
  
    For example, if you distribute copies of such a program, whether
  gratis or for a fee, you must give the recipients all the rights that
  you have.  You must make sure that they, too, receive or can get the
  source code.  And you must show them these terms so they know their
  rights.
  
    We protect your rights with two steps: (1) copyright the software, and
  (2) offer you this license which gives you legal permission to copy,
  distribute and/or modify the software.
  
    Also, for each author's protection and ours, we want to make certain
  that everyone understands that there is no warranty for this free
  software.  If the software is modified by someone else and passed on, we
  want its recipients to know that what they have is not the original, so
  that any problems introduced by others will not reflect on the original
  authors' reputations.
  
    Finally, any free program is threatened constantly by software
  patents.  We wish to avoid the danger that redistributors of a free
  program will individually obtain patent licenses, in effect making the
  program proprietary.  To prevent this, we have made it clear that any
  patent must be licensed for everyone's free use or not licensed at all.
  
    The precise terms and conditions for copying, distribution and
  modification follow.
  
                      GNU GENERAL PUBLIC LICENSE
     TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
  
    0. This License applies to any program or other work which contains
  a notice placed by the copyright holder saying it may be distributed
  under the terms of this General Public License.  The "Program", below,
  refers to any such program or work, and a "work based on the Program"
  means either the Program or any derivative work under copyright law:
  that is to say, a work containing the Program or a portion of it,
  either verbatim or with modifications and/or translated into another
  language.  (Hereinafter, translation is included without limitation in
  the term "modification".)  Each licensee is addressed as "you".
  
  Activities other than copying, distribution and modification are not
  covered by this License; they are outside its scope.  The act of
  running the Program is not restricted, and the output from the Program
  is covered only if its contents constitute a work based on the
  Program (independent of having been made by running the Program).
  Whether that is true depends on what the Program does.
  
    1. You may copy and distribute verbatim copies of the Program's
  source code as you receive it, in any medium, provided that you
  conspicuously and appropriately publish on each copy an appropriate
  copyright notice and disclaimer of warranty; keep intact all the
  notices that refer to this License and to the absence of any warranty;
  and give any other recipients of the Program a copy of this License
  along with the Program.
  
  You may charge a fee for the physical act of transferring a copy, and
  you may at your option offer warranty protection in exchange for a fee.
  
    2. You may modify your copy or copies of the Program or any portion
  of it, thus forming a work based on the Program, and copy and
  distribute such modifications or work under the terms of Section 1
  above, provided that you also meet all of these conditions:
  
      a) You must cause the modified files to carry prominent notices
      stating that you changed the files and the date of any change.
  
      b) You must cause any work that you distribute or publish, that in
      whole or in part contains or is derived from the Program or any
      part thereof, to be licensed as a whole at no charge to all third
      parties under the terms of this License.
  
      c) If the modified program normally reads commands interactively
      when run, you must cause it, when started running for such
      interactive use in the most ordinary way, to print or display an
      announcement including an appropriate copyright notice and a
      notice that there is no warranty (or else, saying that you provide
      a warranty) and that users may redistribute the program under
      these conditions, and telling the user how to view a copy of this
      License.  (Exception: if the Program itself is interactive but
      does not normally print such an announcement, your work based on
      the Program is not required to print an announcement.)
  
  These requirements apply to the modified work as a whole.  If
  identifiable sections of that work are not derived from the Program,
  and can be reasonably considered independent and separate works in
  themselves, then this License, and its terms, do not apply to those
  sections when you distribute them as separate works.  But when you
  distribute the same sections as part of a whole which is a work based
  on the Program, the distribution of the whole must be on the terms of
  this License, whose permissions for other licensees extend to the
  entire whole, and thus to each and every part regardless of who wrote it.
  
  Thus, it is not the intent of this section to claim rights or contest
  your rights to work written entirely by you; rather, the intent is to
  exercise the right to control the distribution of derivative or
  collective works based on the Program.
  
  In addition, mere aggregation of another work not based on the Program
  with the Program (or with a work based on the Program) on a volume of
  a storage or distribution medium does not bring the other work under
  the scope of this License.
  
    3. You may copy and distribute the Program (or a work based on it,
  under Section 2) in object code or executable form under the terms of
  Sections 1 and 2 above provided that you also do one of the following:
  
      a) Accompany it with the complete corresponding machine-readable
      source code, which must be distributed under the terms of Sections
      1 and 2 above on a medium customarily used for software interchange; or,
  
      b) Accompany it with a written offer, valid for at least three
      years, to give any third party, for a charge no more than your
      cost of physically performing source distribution, a complete
      machine-readable copy of the corresponding source code, to be
      distributed under the terms of Sections 1 and 2 above on a medium
      customarily used for software interchange; or,
  
      c) Accompany it with the information you received as to the offer
      to distribute corresponding source code.  (This alternative is
      allowed only for noncommercial distribution and only if you
      received the program in object code or executable form with such
      an offer, in accord with Subsection b above.)
  
  The source code for a work means the preferred form of the work for
  making modifications to it.  For an executable work, complete source
  code means all the source code for all modules it contains, plus any
  associated interface definition files, plus the scripts used to
  control compilation and installation of the executable.  However, as a
  special exception, the source code distributed need not include
  anything that is normally distributed (in either source or binary
  form) with the major components (compiler, kernel, and so on) of the
  operating system on which the executable runs, unless that component
  itself accompanies the executable.
  
  If distribution of executable or object code is made by offering
  access to copy from a designated place, then offering equivalent
  access to copy the source code from the same place counts as
  distribution of the source code, even though third parties are not
  compelled to copy the source along with the object code.
  
    4. You may not copy, modify, sublicense, or distribute the Program
  except as expressly provided under this License.  Any attempt
  otherwise to copy, modify, sublicense or distribute the Program is
  void, and will automatically terminate your rights under this License.
  However, parties who have received copies, or rights, from you under
  this License will not have their licenses terminated so long as such
  parties remain in full compliance.
  
    5. You are not required to accept this License, since you have not
  signed it.  However, nothing else grants you permission to modify or
  distribute the Program or its derivative works.  These actions are
  prohibited by law if you do not accept this License.  Therefore, by
  modifying or distributing the Program (or any work based on the
  Program), you indicate your acceptance of this License to do so, and
  all its terms and conditions for copying, distributing or modifying
  the Program or works based on it.
  
    6. Each time you redistribute the Program (or any work based on the
  Program), the recipient automatically receives a license from the
  original licensor to copy, distribute or modify the Program subject to
  these terms and conditions.  You may not impose any further
  restrictions on the recipients' exercise of the rights granted herein.
  You are not responsible for enforcing compliance by third parties to
  this License.
  
    7. If, as a consequence of a court judgment or allegation of patent
  infringement or for any other reason (not limited to patent issues),
  conditions are imposed on you (whether by court order, agreement or
  otherwise) that contradict the conditions of this License, they do not
  excuse you from the conditions of this License.  If you cannot
  distribute so as to satisfy simultaneously your obligations under this
  License and any other pertinent obligations, then as a consequence you
  may not distribute the Program at all.  For example, if a patent
  license would not permit royalty-free redistribution of the Program by
  all those who receive copies directly or indirectly through you, then
  the only way you could satisfy both it and this License would be to
  refrain entirely from distribution of the Program.
  
  If any portion of this section is held invalid or unenforceable under
  any particular circumstance, the balance of the section is intended to
  apply and the section as a whole is intended to apply in other
  circumstances.
  
  It is not the purpose of this section to induce you to infringe any
  patents or other property right claims or to contest validity of any
  such claims; this section has the sole purpose of protecting the
  integrity of the free software distribution system, which is
  implemented by public license practices.  Many people have made
  generous contributions to the wide range of software distributed
  through that system in reliance on consistent application of that
  system; it is up to the author/donor to decide if he or she is willing
  to distribute software through any other system and a licensee cannot
  impose that choice.
  
  This section is intended to make thoroughly clear what is believed to
  be a consequence of the rest of this License.
  
    8. If the distribution and/or use of the Program is restricted in
  certain countries either by patents or by copyrighted interfaces, the
  original copyright holder who places the Program under this License
  may add an explicit geographical distribution limitation excluding
  those countries, so that distribution is permitted only in or among
  countries not thus excluded.  In such case, this License incorporates
  the limitation as if written in the body of this License.
  
    9. The Free Software Foundation may publish revised and/or new versions
  of the General Public License from time to time.  Such new versions will
  be similar in spirit to the present version, but may differ in detail to
  address new problems or concerns.
  
  Each version is given a distinguishing version number.  If the Program
  specifies a version number of this License which applies to it and "any
  later version", you have the option of following the terms and conditions
  either of that version or of any later version published by the Free
  Software Foundation.  If the Program does not specify a version number of
  this License, you may choose any version ever published by the Free Software
  Foundation.
  
    10. If you wish to incorporate parts of the Program into other free
  programs whose distribution conditions are different, write to the author
  to ask for permission.  For software which is copyrighted by the Free
  Software Foundation, write to the Free Software Foundation; we sometimes
  make exceptions for this.  Our decision will be guided by the two goals
  of preserving the free status of all derivatives of our free software and
  of promoting the sharing and reuse of software generally.
  
                              NO WARRANTY
  
    11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
  FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
  OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
  OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
  TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
  PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
  REPAIR OR CORRECTION.
  
    12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
  WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
  REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
  INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
  OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
  TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
  YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
  PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGES.
  
                       END OF TERMS AND CONDITIONS
  
              How to Apply These Terms to Your New Programs
  
    If you develop a new program, and you want it to be of the greatest
  possible use to the public, the best way to achieve this is to make it
  free software which everyone can redistribute and change under these terms.
  
    To do so, attach the following notices to the program.  It is safest
  to attach them to the start of each source file to most effectively
  convey the exclusion of warranty; and each file should have at least
  the "copyright" line and a pointer to where the full notice is found.
  
      <one line to give the program's name and a brief idea of what it does.>
      Copyright (C) <year>  <name of author>
  
      This program is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 2 of the License, or
      (at your option) any later version.
  
      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
  
      You should have received a copy of the GNU General Public License along
      with this program; if not, write to the Free Software Foundation, Inc.,
      51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
  
  Also add information on how to contact you by electronic and paper mail.
  
  If the program is interactive, make it output a short notice like this
  when it starts in an interactive mode:
  
      Gnomovision version 69, Copyright (C) year name of author
      Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
      This is free software, and you are welcome to redistribute it
      under certain conditions; type `show c' for details.
  
  The hypothetical commands `show w' and `show c' should show the appropriate
  parts of the General Public License.  Of course, the commands you use may
  be called something other than `show w' and `show c'; they could even be
  mouse-clicks or menu items--whatever suits your program.
  
  You should also get your employer (if you work as a programmer) or your
  school, if any, to sign a "copyright disclaimer" for the program, if
  necessary.  Here is a sample; alter the names:
  
    Yoyodyne, Inc., hereby disclaims all copyright interest in the program
    `Gnomovision' (which makes passes at compilers) written by James Hacker.
  
    <signature of Ty Coon>, 1 April 1989
    Ty Coon, President of Vice
  
  This General Public License does not permit incorporating your program into
  proprietary programs.  If your program is a subroutine library, you may
  consider it more useful to permit linking proprietary applications with the
  library.  If this is what you want to do, use the GNU Lesser General
  Public License instead of this License.
  

  2. pybind11

  Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>, All rights reserved.
  
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  
  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.
  
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
  
  3. Neither the name of the copyright holder nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
  Please also refer to the file .github/CONTRIBUTING.md, which clarifies licensing of
  external contributions to this project including patches, pull requests, etc.


--------------------------------------------------------------------------------
https://www.siedler25.org                Copyright (C) 2005-2022 Settlers Freaks
--------------------------------------------------------------------------------
