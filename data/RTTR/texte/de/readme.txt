                              RETURN TO THE ROOTS
--------------------------------------------------------------------------------

A. Allgemeine Hinweise
B. Installation
  1. Windows
  2. Linux
  3. Mac OSX
  4. Settings-Ordner
C. Spiel
  1. Erstellen eines Spiels
  2. Multiplayer-Spiele
  3. Replays
  4. Optionen
D. Abstürze und Fehler
E. Übersicht: Updates und Änderungen
F. Lizenzen

--------------------------------------------------------------------------------

A. Allgemeine Hinweise

  Das Spiel benötigt eine OpenGL2.0-fähige Grafikkarte mit mindestens
  64 MB Grafikspeicher. Ein Prozessor mit 800 MHz reicht aus.

  Weiterhin benötigt man eine installierte "Siedler 2 Gold-Edition" oder die
  Originalversion + Missions-CD.

--------------------------------------------------------------------------------

B. Installation

  1. Windows

    Windows 7 und 10 werden unterstüzt. Windows Vista und älter können
    funktionieren, werden aber nicht offiziell unterstützt.

    Zum Spielen sucht man die Verzeichnisse "DATA" und "GFX" im Original
    "Die Siedler 2 Gold" Spiel (oder S2 + Mission CD) und kopiert diese in das
    Nightly-Verzeichnis (in den Ordner wo sich auch die Datei
    "Put your S2-Files in here" befindet). Diese können sowohl von einer
    Installation als auch direkt von der CD kommen.

    Alternativ können symbolische Verknüpfungen als Admin erstellt werden:
    mklink /D DATA "C:\S2\DATA"
    mklink /D GFX "C:\S2\GFX"

    Darin kann man dann s25client.exe zum Spielen von Siedler II.5 RTTR
    ausführen.

    (In Nightlies kann man mit RTTR.BAT updaten.)

    Es werden KEINE originalen Spieldateien verändert. Man kann weiterhin
    das Original ohne Beeinträchtigung spielen.

  ------------------------------------------------------------------------------

  2. Linux

    Zunächst benötigt man folgende Pakete
    (ggf über Paketmanager/per Hand installieren):

    libsdl2 libsdl-mixer2 gettext

    Weiterhin sollte man sicherstellen, dass DirectRendering funktioniert:

    glxinfo | grep direct

    Wenn die Ausgabe "direct rendering: Yes" kommt, ist alles in Ordnung,
    ansonsten kann es sein, dass man eine miese Spielperformance hat. In
    dem Fall muss die Grafikbeschleunigung geprüft werden.

    Die eigentliche Installation:

    z.B. nach /opt/s25rttr entpacken:

    mkdir -p /opt/s25rttr
    cd /opt/s25rttr
    tar -jxvf s25rttr_*.tar.bz2

    Nun muss man entweder nur noch die original "Siedler 2"-Installation
    nach /opt/s25rttr/share/s25rttr/S2 kopieren, oder einen Symlink dorthin
    anlegen.

    Starten kann man das Ganze dann mit
    /opt/s25rttr/bin/rttr.sh

    (Nightly Versionen updaten sich selber. Wenn man rttr.sh mit der
    Option noupdate startet, wird kein Update durchgeführt.)

  ------------------------------------------------------------------------------

  3. Mac OSX

    Das Spiel kann direkt aus dem App-Bundle gestartet werden.

  ------------------------------------------------------------------------------

  4. Settings-Ordner

    Der Settings-Ordner befindet sich unter:

    Windows: <Benutzer-Ordner>/My Games/Return To The Roots
    Linux:   ~/.s25rttr
    Mac OSX: ~/Library/Application Support/Return To The Roots

    Er wird beim ersten Start erstellt, sofern er nicht existiert.
    Alle Logs, Einstellungen, Savegames und Replays befinden sich
    in den jeweiligen Unterordnern.

--------------------------------------------------------------------------------

C. Spiel

  1. Erstellen eines Spiels

    Es gibt sowohl einen Einzelspieler- als auch einen Mehrspieler-Modus.
    Die Einzelspieler-Modi sind unter dem entsprechenden Menüpunkt zu
    finden und weitestgehend selbsterklärend. Man kann auch alleine im
    Mehrspieler-Modus spielen:

    1. Im Hauptmenü auf Mehrspieler gehen
    2. Direkte IP
    3. Spiel erstellen
    4. Im folgenden Fenster einen Namen für das Spiel eingeben (Passwort
       kann weggelassen werden).
    5. In der Kartenauswahl kann oben eine Kategorie ausgewählt
       werden, ähnlich wie im Original.
       Alternativ kann unten über "Spiel laden" ein Spielstand geladen
       werden.
    6. Nach der Auswahl der Karte und einem Klick auf "Weiter" gelangt man
       ins Host-Menü, wo im oberen Teil die einzelnen Spieler samt ihrer
       Eigenschaften aufgelistet sind.
       Achtung: Um das Spiel starten zu können, müssen entweder alle Plätze
       mit menschlichen oder KI-Spielern besetzt werden, oder die Slots
       müssen geschlossen werden (mit Klick auf die Buttons unter
       "Spielername").

    Im Einzelspieler-Modus klickt man auf "Endlosspiel" und folgt dann
    den obigen Schritten ab Schritt 5.

  2. Multiplayer-Spiele

    a) Direktes Spielen
       Die Erstellung eines Multiplayerspiels erfolgt genauso wie bei 1. Die
       weiteren Spieler gehen bei "Direkte IP" unter "Spiel beitreten" und
       geben die IP oder den Hostnamen des Spielerstellers (Host) ein.

    b) Spielen über integrierte Lobby
       Über die eingebaute Lobby kann man vorhandene Spiele einsehen,
       diese eröffnen und auch beitreten. Weiterhin kann man sich hier
       mit anderen Spielern direkt unterhalten.

       Es wird ein Foren-Account von http://www.siedler25.org benötigt, dessen
       Login-Daten für den Login in die Spiel-Lobby verwendet werden.

    c) LAN Spiele
       Der LAN-Bereich ist ähnlich der Internet-Lobby, funktioniert aber über
       das lokale Netz und benötigt keinen Login. Es können auch Programme
       wie Hamachi verwendet werden, um virtuelle LANs über das Internet
       zu erstellen.

    Für das Spiel wird Port 3665 (TCP) genutzt. Dieser muss, falls man selbst
    das Spiel eröffnen möchte, u.a. bei Verwendung eines Routers, einer
    Firewall usw. freigegeben werden!
    Router nennen diese Einstellung "Virtual Server" oder einfach nur
    "Port Forwarding". Dort ist dann Port 3665 vom Typ TCP auf die interne IP
    des Spiel-PCs weiterzuleiten. Bei der Verwendung einer lokalen Firewall
    muss auch hier der Zugriff zugelassen werden.
    Im LAN Modus werden die Ports 3666 und 3667 (UDP) zum Finden von Spielen
    verwendet.

    Es ist nicht notwendig, dass jeder Spieler die Karte hat. Diese wird bei
    Spielbegin automatisch vom Host übertragen.

    Die Schneckensymbole im oberen Teil stehen für Lags der jeweiligen
    Spieler.

    Wenn der Host das Spiel verlässt können die übrigen Spieler das Spiel
    nicht weiter fortführen.

    Zum Pausieren/Fortführen dient die Taste "P" (nur vom Host möglich).
    Zum Chatten muss man "Enter" drücken, dann öffnet sich ein Chatfenster.

  3. Replays

    Replays sind Aufzeichnungen von gespielten Partien. Sie können unter
    "Einzelspieler", "Replay abspielen" angesehen werden.

    Mit den Tasten [+] und [-] kann die Geschwindigkeit erhöht bzw.
    verringert werden. Mit der Taste "J" ist es möglich, Abschnitte im
    Replay zu überspringen.

    Alle Replays sind im Replays Ordner des Settings-Ordner zu finden.

  4. Speichern

    Speichern ist im Spielmenü an der "originalen" Stelle möglich.
    Speichern ist auch aus dem Replaymodus an beliebiger Stelle möglich.
    Laden von gespeicherten Spieleständen erfolgt im Mehrspieler-Modus
    über die Kartenauswahl und im Einzelspieler-Modus über den
    entsprechenden Menüpunkt.
    Mitspieler benötigen kein eigenes Savegame, dieses wird automatisch
    beim Verbinden übertragen.

  5. Optionen

    Die Einstellungen im Optionsmenü sollten so weit selbsterklärend sein.


--------------------------------------------------------------------------------

D. Abstürze und Fehler

  Wir arbeiten hart daran das Spiel immer weiter zu verbessern. Leider
  lassen sich Abstürze, Asyncs, Speicherzugriffs- und sonstige Fehler
  nicht ganz ausschließen.

  Durch Replays können Fehler leicht rekonstruiert werden. Ihr könnt uns
  mit dem Zusenden euer Replays helfen, weitere zu finden. Bitte stellt
  dabei sicher, dass ihr immer die neuste Nightly-Version verwendet.
  Fehler aus veralteten Versionen sind mitunter schon lange behoben.

  Falls ein Async aufgetreten ist, findet ihr im LOGS-Verzeichnis
  Log-Dateien. Schickt uns diese bitte von JEDEM(!) Spieler einschließlich
  eines Replays (Verzeichnis REPLAYS).
  Bitte stellt sicher, dass ihr den Async nicht absichtlich durch Spielen mit
  verschiedenen Versionen oder Cheaten herbeigeführt habt!

  Wenn ihr Fehler findet, bitte diese in Github posten:

      https://github.com/Return-To-The-Roots/s25client

  Github erhöht die Übersichtlichkeit für Bugs und ermöglicht euch zudem
  selber einsehen zu können, wann der Bug gefixt wurde. Außerdem bietet
  es eine gute Möglichkeit der Kommunikation zwischen uns und euch, vor
  allem wenn wir noch weitere Informationen von euch benötigen.

  Alternativ könnt ihr euch auch im Forum oder im IRC-Channel unter
  irc.freenode.net:6667/#siedler2.5 melden.
  Ihr könnt dem Chat auch über unsere Seite beitreten.

  Vielen Dank!

  Settlers Freaks
  7. Juli 2017

--------------------------------------------------------------------------------

E. Übersicht: Updates und Änderungen

  * 0.9.4 - 06.01.2022
  ------------------------------------------------------------------------------
  - Verschiedene Fixes für Bugs, die nicht-ladbare Savegames und Crashes verursachten
  - Fix für bisuelle glitches bei hohem Terrain
  - Drücken von ESC speichert die Einstellungen vor dem Schließen des Fensters
  - Fix für das nicht-schließbar Action-Window
  - Bei Spielstart werden die zuletzt geöffneten Fesnter wieder geöffnet und deren Position wiederhergestellt
  - Fix fehlerhafte Behandlung der Versionen (behebt Anzeigefehler und Fehler bei Beitritt zu anderen Spielern)

  * 0.9.1 - 24.07.2021
  ------------------------------------------------------------------------------
  - Vollbild Modus für alle Treiber und Betriebssystem
  - Random map generator
  - Sonderzeichen im Benutzernamen werden unterstüzt
  - Einige Fehler, Abstürze und Asyncs behoben
  - Map editor
  - Performanceverbesserungen

  * 0.8.2 - 22.08.2017
  ------------------------------------------------------------------------------
  - Viele Async Fixes
  - Nutzung von UTF8 um mehr Sprachen zu unterstützen
  - Viele neue Addons
  - Unterstützung aller Terrains aus S2
  - Zoom Funktion
  - Lua Scripting
  - LAN Lobby
  - Verbesserung der Code Qualität

  * 0.8.1
  ------------------------------------------------------------------------------
  - Fehlerkorrekturen
  - Funktionierende KI
  - Vollständige Seefahrt

  * 0.8.0
  ------------------------------------------------------------------------------
  - Starke Verbesserung der KI
  - Viele Bugfixes
  - Verbesserung der Geschwindigkeit
  - Beobachtungsfenster

  * 0.7.2 - 17.01.2011 *
  ------------------------------------------------------------------------------
  - Kritische Fehlerkorrekturen

  * 0.7 - Seventh version - 24.12.2010 *
  ------------------------------------------------------------------------------
  - OpenSource!
  - Übersetzung: Niederländisch
  - Übersetzung: Russisch (Zeichensatz fehlt)
  - Übersetzung: Czech
  - Übersetzung: Estonian
  - Übersetzung: Italian
  - Übersetzung: Norwegian
  - Übersetzung: Polish
  - Übersetzung: Slovenian
  - Übersetzung: Slovak
  - Statistik
  - Post
  - Diplomatie - noch nicht fertig
  - Erste KI von jh
  - Seefahrt (noch nicht fertig)
  - Addon Menü

  * 0.6 - Sechste Version - 25.01.2009 *
  ------------------------------------------------------------------------------
  - viele Bugs behoben
  - Fog of War (mit Teamview-Option)
  - Spähturm
  - Minimap
  - Mapvorschau im Hostmenü
  - Planierer
  - Balancing
  - Trägeranimationen hinzugefügt
  - Mehrsprachenunterstützung
  - Übersetzung: Spanisch
  - Übersetzung: Ungarisch
  - Übersetzung: Schwedisch
  - Übersetzung: Französisch (nicht abgeschlossen)
  - Übersetzung: Finnisch (nicht abgeschlossen)
  - Siegesmeldungen
  - Tastaturbefehle + Readme
  - Option im Hostmenü: Abriss-Verbot
  - Gebäude Info
  - Einige Hausanimationen hinzugefügt

  * 0.5 - Fünfte Version - 27.01.2008 *
  ------------------------------------------------------------------------------
  - sehr viele Bugs behoben
  - Eselstraßen, Eselzüchter
  - Bootsstraßen mit Booten und Werft (baut nur Boote)
  - Bergstraßen
  - Katapulte
  - Soldatenverhalten verändert, rücken nicht mehr so schnell nach
  - beim Abbrennen von Lagerhäusern und dem Hauptquartier flüchten
   nun alle Leute nach draußen wie im Original
  - Async-Logdatei
  - diverse Kleinigkeiten wie ein das Original-Abfragefenster vor dem
   Abbrennen von Gebäuden oder fehlende/falsche Tooltips
  - Sound an/aus Buttons im Spiel

  * 0.4 - Vierte Version - 09.10.2007 *
  ------------------------------------------------------------------------------
  - Speichern und Laden von Spielen eingebaut, inklusive Autosave
  - zig zum Teil schwere Bugs behoben

  * 0.3 - Dritte Version Fix01 -  13.09.2007 *
  ------------------------------------------------------------------------------
  - kritischen Absturz behoben

  * 0.3 - Dritte Version - 12.09.2007 *
  ------------------------------------------------------------------------------
  - diverse Absturzbugs behoben
  - Siedler warten nun, falls der Platz vor ihnen besetzt ist
    (durch Kämpfe z.B.)
  - Settings unter Linux im Home-Verzeichnis untergebracht
  - Settings in lesbares Format gebracht (GER-File)
  - Build-System unter Linux von autotools auf CMake umgestellt
  - CIA-Bot für den IRC aktiviert
  - Lobbygrundfunktionen fertiggestellt
  - Vorbereitungen für Speichern und Laden getroffen

  * 0.2 - Zweite Version - 15.07.2007 *
  ------------------------------------------------------------------------------
  - diverse Bugs behoben und diverse Verzögerungsbugs unterdrückt
  - Menüs in variabler Auflösung
  - Prüfen der Synchronität und Rausschmiss asynchroner Spieler
  - Inventurfenster eingebaut
  - "Nicht Einlagern"/"Auslagern" möglich
  - Bereit-Button im Host-Menü
  - Verteilung korrigiert
  - diverse fehlende Zierobjekte (z.B Stalagmiten, Ruinen) eingebaut
  - zu allen Gebäuden kann nun gesprungen werden
  - einige Zierobjekte werden beim Wegbau nun abgerissen
  - Produktivitätsanzeige und Namenanzeige (C, S) teilweise schon korrekt
   implementiert
  - Abrissbestätigung eingebaut
  - RoadWindow schließt sich nun, wenn man woanders hinklickt

  * 0.1 - Erster Release - 01.07.2007 *
  ------------------------------------------------------------------------------
  - alles ;-)

--------------------------------------------------------------------------------

F. Lizenzen

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
