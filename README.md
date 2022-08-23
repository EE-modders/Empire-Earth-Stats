# 📊 Empire Earth Stats
A brand new way to get EE statistics! Open-Source and [GDPR](https://ec.europa.eu/info/law/law-topic/data-protection/data-protection-eu_en) compliant! \
By [EnergyCube](https://github.com/EnergyCube) for the Empire Earth Community.

🔐 All data is transferred to the server via TLS v1.3\
📡 End of the Computer's IP is hidden\
🔑 Unique Computer ID is protected with SHA512

## What is collected ?
💻 Windows Version\
⏰ Time Played\
🍷 Wine Detection\
🗺 County and City\
⚙ Hardware Informations\
🔧 Compatibility Options\
And some other useful infos to improve the compatibility of the game and follow the evolution of the game.

## 🔨 Build
Require cURL x86 ! (tested with v7.84) \
In EEStats\EEStats\curl place headers and libs (Release: libcurl_a.lib and Debug: libcurl_a_debug.lib)

## 🧾 Note
cURL seems to require Windows Vista or 7 SP1 (I can't test), using boost with XP feature set could be a good idea.

## ⚠️ Known Limitation
If the computer is in standby mode, the calculated time will include the standby time. \
Doing a file checksum on the game binary seems to give bad result (probably because the game is executed?)

## 💡 Ideas / TODO
Fix [Known Limitation](https://github.com/EE-modders/Empire-Earth-Stats#-known-limitation) \
Detect install way (Sierra, Community Setup, GOG, etc...) and report the setup version

## 📖 License
[GNU General Public License v3.0](https://github.com/EE-modders/Empire-Earth-Launcher/blob/main/LICENSE)

## ❤️ Credit
zocker_160
cURL (License: https://github.com/curl/curl/blob/master/COPYING, Website: https://curl.se/) \
sha512.h (License: BSD, Author: Stefan Wilhelm), sha1.h (License: "public domain", Author: Steve Reid)