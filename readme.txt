ZeroVSH Patcher v0.2 - NightStar3 and codestation

ZeroVSH Patcher is a plugin that allows users to customize their PSP to the 
fullest extent without writting to the flash0. ZeroVSH Patcher redirects the
loading of various file types from flash to your memory stick or internal storage, 
thus removing the fear of bricking.

---- How to Use ----

Copy zerovsh_patcher.prx and zerovsh.ini to your seplugins folder

Just place all the files you want to load from your memory stick or internal storage in the 
folder which you have RedirPath set to in zerovsh.ini (Default is (root):/PSP/VSH) and let the plugin do the rest of the work


---- Enabling the clock and calendar ----
Open up zerovsh.ini and set SlidePlugin to Enabled

Then, navigate to the /slide folder and place the files slide_plugin_XXX.rco and slide_plugin_XXX.prx in the folder which you have
RedirPath set to in zerovsh.ini (Default is (root):/PSP/VSH) and rename them to slide_plugin.rco
and slide_plugin.prx

In order to load and stop it, press the HOME button in the XMB

---- Bugs ----

Prompt for disabling bluetooth when using usb or camera with clock and calendar enabled
No umd or umd update icon appearing in XMB with clock and calendar enabled
Taking more than one press to load the clock and calendar (just go to reset device in vsh menu and it should work fine)
Unable to load internet browser with clock and calendar enabled (just go to reset vsh in vsh menu and it should work fine)
Unable to load clock and calendar after using internet browser (just go to reset vsh in vsh menu and it should work fine)

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
[+] Support for PSPGO's clock and calendar on other models
[+] dat file support
[+] Added config support: zerovsh.ini
v0.1:
[+]First release.
