THE MANA PLUS CLIENT
===============

 Version: 1.9.3.23        Date: 2019-03-23

 Development team:
  - See AUTHORS file for a list

 Build status:
  ManaPlus CI: https://git.themanaworld.org/mana/plus/badges/master/pipeline.svg
  Windows: https://gitlab.com/themanaworld/manaplus/docker-windows-builder/badges/master/pipeline.svg
  AppImg: https://gitlab.com/themanaworld/manaplus/appimg-builder/badges/master/pipeline.svg
  deb: https://gitlab.com/themanaworld/manaplus/appimg-builder/badges/master/pipeline.svg
  Coverage: https://git.themanaworld.org/mana/plus/badges/master/coverage.svg
  Doxygen: http://stats.manaplus.org/docs/index.html
  Coverity: https://scan.coverity.com/projects/manaplus

 Dependencies:

  - SDL, SDL_image, SDL_mixer, SDL_ttf, SDL_net (Media framework), SDL_gfx
  - libxml2 or pugixml or tinyxml2 (XML parsing and writing)
  - libcurl (HTTP downloads)
  - libpng (save screenshots)
  - zlib (Archives)

 Optional dependencies:

  - gettext (translations)

 Embedded libs:

  - catch and doctest for unit testing.
  - nvwa for debug memory leaks.
  - SaferCPlusPlus for debug issues in std::vector usage.

 Additiona run time deps:
  - xsel (for clipboard access, used only in SDL 1.2)
  - xdg-open (from xdg-utils, for open url in browser)

 Images and fonts sources:
  - data/icons
  - data/fonts/src


0. Index
--------

1. Account
2. Commands
3. Skills
4. Support
5. License

1. Account
----------

To create an account you can usually press the "Register" button after
choosing your server. When this doesn't work, visit the website of the server
you'd like to register on, since they may be using an online registration
form instead.

2. Commands
-----------

KEYBOARD:

Use arrow keys to move around. Other keys:

- Ctrl               attack
- F1                 toggle the online help
- F2                 toggle profile window
- F3                 toggle inventory window
- F4                 toggle equipment window
- F5                 toggle skills window
- F6                 toggle minimap
- F7                 toggle chat window
- F8                 toggle shortcut window
- F9                 show setup window
- F10                toggle debug window
- Alt + 0-9,-, etc   show emotions.
- S                  sit down / stand up.
- F                  toggle debug pathfinding feature (change map mode)
- P                  take screenshot
- R                  turns on anti-trade function.
- A                  target nearest monster
- H                  hide all non-sticky windows
- Z                  pick up item
- Enter              focus chat window / send message

MOUSE:

Left click to execute default action: walk, pick up an item, attack a monster
and talk to NPCs (be sure to click on their feet). Right click to show up a
context menu. Holding [Left Shift] prevents from walking when attacking.

/Commands:

Whispers:
- /closeall          close all whispers.
- /ignoreall         add all whispers to ignore list.
- /msg NICK text
- /whisper NICK text
- /w NICK text       send whisper message to nick.
- /query NICK
- /q NICK            open new whisper tab for nick.

Actions:
- /help              show small help about chat commands. /target NICK - select nick as target. Can be monster or player nick.
- /outfit N          wear outfit number N.
- /outfit next       wear next outfit.
- /outfit prev       wear previous outfit.
- /emote N           use emotion number N.
- /away
- /away MSG          set away mode.
- /follow NICK       start follow mode.
- /imitation NICK    start imitation mode.
- /heal NICK         heal nick.
- /move X Y          move to X,Y position in short distance.
- /navigate x y      move to position x,y in current map in any distance.
- /mail NICK MSG     send offline message to NICK. Working only in tmw server.
- /disconnect        quick disconnect from server.
- /attack            attack target.
- /undress NICK      remove all clothes from nick. Local effect only.

Trade:
- /trade NICK        start trade with nick.
- /priceload         load shop price from disc.
- /pricesave         save shop price to disc.

Player relations:
- /ignore NICK       add nick to ignore list.
- /unignore NICK     Remove nick from ignore list.
- /friend NICK
- /befriend NICK     add nick to friends list.
- /disregard NICK    add nick to disregarded list.
- /neutral NICK      add nick to neutral relation list.
- /erase NICK        add nick to erased list.
- /clear             clear current chat tab.
- /createparty NAME  create party with selected name.
- /me text           send text to chat as /me command in irc.

Debug:
- /who               print online players number to chat.
- /all               show visible beings list in debug tab.
- /where             print current player position to chat.
- /cacheinfo         show text cache info.
- /dirs              show client directories in debug window.

Other:
- /help              Displays the list of commands
- /announce          broadcasts a global msg(Gm Cammand only)
- /who               shows how many players are online
- /where             displays the map name your currently on

4. Support
----------

If you're having issues with this client, feel free to report them to us.
You can report on forum (http://forums.themanaworld.org/viewforum.php?f=12),
bugtracker (https://git.themanaworld.org/mana/plus/issues/) or IRC on irc.freenode.net in the #germantmw channel.

If you have feedback about a specific game that uses the ManaPlus client, be sure
to contact the developers of the game instead.

5. Licenses
-----------

All code in this repository under GPL 2+ license. Art can be under CC-BY-SA or GPL 2+.

Github, Bitbucked or other non free hostings may use strange TOS what may violate this licenses.
If any of this hostings want to violate GPL or/and CC-BY-SA licenses,
they not have any rights to do so.
This hostings have rights to remove ManaPlus repositories if they want violate given licenses.

ManaPlus main repository hosted on https://gitlab.com/manaplus/manaplus/
