Summary
-------

clementinewm is a very simple and lightweight window manager.


Status
------
 * 27.10.14 -> v0.0.7.1
 
  -Realive this window manager to runs in 2014

  -Deleted keybindings file "keys"

  -Second button of mouse not kills the wm, runs dmenu_run

  -Fixed the route for themes "look" to ~./config/look

  -Added Missing headers to compile ok now.. 
 
  -Fixed crashes in wm, with gimp, and other windows...
   
In debian and derivatives: 

    # apt-get install build-essential libx11-dev

If use slackware, archlinux, headers is present now, in default installation...

Need Xlib, then:

    $ make
    
    # make install

    $ make clean

To install looks:

    In the current dir of clementinewm:
    
    $ cp look $HOME/.config/look

    The "default" look is black, file "look"

Bugs
----
 * No bugs for the moment ;) (I mean, no importants bugs ;)

Todo
----
 * Fixes crashes of this wm (rarely)

If you have some particular request, just send me an e-mail, and I will see for it!

Here a screenshot: 

License
-------

Licensed under GNU GPL version 2, see [LICENSE][law] file for more copyright and license information.

  [law]: https://raw.githubusercontent.com/djmasde/clementinewm/master/COPYING

Thanks
------

 * [Clementine original homepage](http://clementine.sourceforge.net/)