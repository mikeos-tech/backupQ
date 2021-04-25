# backupQ
A small C++ command-line utility to take questions from the PHP building a website and feed answers back based on database content.

This is part of what will be a bigger project - building a Music/Media Server.

## Music Server Project

This is part of my project to build a music/media server.  My intention is to build and document the server build, keeping the system modular, so that it's easy to add additional components and ignore the components that you don't require.

This project is Linux based, I have used Ubuntu Server (https://ubuntu.com/download/server), but I will try and keep things fairly generic so with a little adaption it should work on other Linux distributions.

I have been using a Vortexbox server (https://www.vortexbox.org/) for a long time and have loved using it.  This is essentially a Linux distribution with a very specific purpose, but it no longer supported/kept up to date. This has inspired me to building my own alternative.  I originally used LMS with Logitech audio components, but currently us Raspberry Pi with sound-cards as players.

The Vortexbox build uses LMS (Logitech Media Server) (https://en.wikipedia.org/wiki/Logitech_Media_Server) for playing the music, so I'm continuing to use that as a core element of my system and building it onto Ubuntu server.

Components:
- Ubuntu Server (https://ubuntu.com/download/server).  A current/commonly used Linux distribution.
- LMS (https://en.wikipedia.org/wiki/Logitech_Media_Server) - playing Audio files
- Nginx (https://www.nginx.com/) - for Web interface/server
- Smarty (https://www.smarty.net/) - Web page templating 
- Transmission (https://transmissionbt.com/) - bit torrent (using web interface, for downloads on server)
- Webmin (https://www.webmin.com/) - Server Admin tools
- get_iplayer - Downloading TV programmes - I've automated the download process, making it server based.
- PHP - to build the dynamic web pages
- SQLite - various databases for tracking.
- BASH - scripts for server maintenance
- HTML/CSS - Web Pages
- C++ Small amount of utility coding
- MAL - A web interface to the LMS database, providing Artist, Album and Track lists/views - I developed this myself and will make it available when I've migrated it to smarty 

I'm not claiming any expertise, that how I do this, will be the only or the best way to achieve it - *it probably won't be!*

I am hoping that you will be inspired to experiment and to learn, what would you like to have your server do?

As part of this I'm using and learning tools like *vim*, *tmux*, *ssh*, *git* and github.  I've been using Linux for a long time but have been working mainly within the KDE desktop GUI recently, I enjoying getting back to the command line, learning a lot.

Mike



