ZeroVSH Patcher v0.2 - NightStar3 and codestation

ZeroVSH Patcher is a plugin that allows users to customize their PSP to the 
fullest extent without writting to the flash0. ZeroVSH Patcher redirects the
loading of various file types from flash to your memory stick or internal storage, 
thus removing the fear of bricking.

In version 0.2, there is support for the psp go's clock and calendar on other psp models (currently psp 1k is not supported)
Please scroll down for further information. 


---- How to Use ----

Copy zerovsh_patcher.prx and zerovsh.ini to your seplugins folder

Just place all the files you want to load from your memory stick or internal storage in the 
folder which you have RedirPath set to in zerovsh.ini (Default is (root):/PSP/VSH) and let the plugin do the rest of the work


---- Enabling the clock and calendar ----

------------------------------
You must be on a psp model that isn't a 1k, due to the fact that there is a memory bug.
We will try to get this fixed in version 0.3
------------------------------

Open up zerovsh.ini and set SlidePlugin to Enabled

Then, navigate to the /slide folder and place the files slide_plugin_XXX.rco, slide_plugin_XXX.prx, and netconf_bt_plugin_6XX.prx(only if 6.3X and above) in the folder which you have
RedirPath set to in zerovsh.ini (Default is (root):/PSP/VSH) and rename them to slide_plugin.rco, slide_plugin.prx, and netconf_bt_plugin.prx

NOTE: If you have a umd inserted
-----------> If the loading icon (small animation going on in the bottom right corner of your screen) is visible, wait for it to pass before
		trying to trigger the clock and calendar

In order to load it wait 1 second(and do not press any buttons during that time period), and press the HOME button
In order to stop it, just press the HOME button again.


---- Bugs ----

-Prompt for disabling bluetooth when using usb or camera with clock and calendar enabled
-Umd, and umd update shows a blank image with clock and calendar enabled
-Does not work on 1k psp

(Feel free to report new bugs)


---- For Developers ----

You must have the pspsdk or psptoolchain set up in order to compile the plugin
Just run the script in the trunk corresponding to your operating system, and it should compile fine

The plugin will be compiled to the /bin folder in the trunk


-- Note --

 If you have a patch you want to submit, contact one of the active developers and we shall add your
patch to the svn, and add your name to the credits

If you are interested in becoming an active developer for the project, contact one of the project leaders
and we shall give your rights to the svn, if you qualify

You can download the source code from http://code.google.com/p/zerovsh-patcher/

------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Check @Zer0Shad0w / @codestation on Twitter for updates.

------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CFW/HEN's supported:

* M33
* TN
* PRO
* ME
* GEN
* more, but untested


File types supported:

* Fonts - .pgf
* Bitmap Images - .bmp
* Resources - .rco
* Gameboots - .pmf
* Modules - .prx
* default theme - .dat
* (more to come hopefully)


Changelog
v0.2a:
[+] Support for PSPGO's clock and calendar on other models (non-1k atm)
[+] Added 5.70 support
[+] dat file support
[+] Added config support: zerovsh.ini
v0.1:
[+]First release.
