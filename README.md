Summary
-------

clementinewm is a very simple and lightweight window manager.

is a "fork" of the original wm (Clementine Window Manager), with bugfixes


Status
------
 * 29.10.14
  
  -Fixing focus, in firefox or another windows...
  
  -minor fixes in windowsystem.cpp

 * 28.10.14 -> v0.0.7.1

  -Add a command line options: -v, for version, and -exit, for exit the wm.

 * 27.10.14
 
  -Realive this window manager to runs in 2014

  -Deleted keybindings file "keys"

  -Second button of mouse not kills the wm, runs dmenu_run

  -Fixed the route for themes "look" to ~./config/look

  -Added Missing headers to compile ok now.. 
 
  -Fixed crashes in wm, with gimp, and other windows...
 
  -Renamed the executable to clementinewm "conflicts with clementine music player"   
   
In debian and derivatives: 

    # apt-get install build-essential libx11-dev

If use slackware, archlinux, headers is present now, in default installation...

Need Xlib, then:

    $ make
    
    # make install

    $ make clean

To install looks, in the current dir of clementinewm:
    
    $ cp look $HOME/.config/look

    The "default" look is black, file "look"

Bugs
----
 * No bugs for the moment ;) (I mean, no importants bugs ;)

Todo
----
 * Fixes crashes of this wm (rarely)

If you have some particular request, just send me an e-mail, and I will see for it!

Here a screenshot: http://a.pomf.se/frnjkk.png

License
-------

Licensed under GNU GPL version 2, see [LICENSE][law] file for more copyright and license information.

  [law]: https://raw.githubusercontent.com/djmasde/clementinewm/master/COPYING

Thanks
------

 * [Clementine original homepage](http://clementine.sourceforge.net/)
