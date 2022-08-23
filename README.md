![Visitor count](https://shields-io-visitor-counter.herokuapp.com/badge?page=EE-modders.Empire-Earth-Stats)
[![GitHub stars](https://img.shields.io/github/stars/EE-modders/Empire-Earth-Setup)](https://github.com/EE-modders/Empire-Earth-Stats/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/EE-modders/Empire-Earth-Setup)](https://github.com/EE-modders/Empire-Earth-Stats/network)
[![Setup Version](https://img.shields.io/badge/Empire%20Earth%20Stats%20Version-v1.0.0-blue)](https://github.com/EE-modders/Empire-Earth-Stats)
# 📊 Empire Earth Stats
A brand new way to get EE statistics! Open-Source and [GDPR](https://ec.europa.eu/info/law/law-topic/data-protection/data-protection-eu_en) compliant! \
By [EnergyCube](https://github.com/EnergyCube) for the Empire Earth Community.

🔐 All data is transferred to the server via TLS v1.3\
📡 End of the Computer's IP is hidden\
🔑 Unique Computer ID is protected with SHA512

## 🔍 What is collected ?
💻 Windows Version\
⏰ Time Played\
🍷 Wine Detection\
🗺 County and City\
⚙ Hardware Informations\
🔧 Compatibility Options\
And some other useful infos to improve the compatibility of the game and follow the evolution of the game.

## ⚖️ Your data rights
You can contact EnergyCube at any time on Discord with EnergyCube#7471 or by email with dev@energycube.fr to ask to have access to all the data concerning you and/or to ask for their deletion by providing your computer identifier available in the DLL logs in the game folder in "EEStats.log". \
All the communicated data are the ones you can see manipulated in the code, feel free to build the dll and use it in debug mode to see all the communicated data in clear during execution (disabled for release for obvious privacy reasons!)

## 🔨 Build
Require cURL x86 ! (tested with v7.84) \
In EEStats\EEStats\curl place headers and libs (Release: libcurl_a.lib and Debug: libcurl_a_debug.lib)

## 🧾 Note
cURL seems to require Windows Vista or 7 SP1 (I can't test), using boost with XP feature set could be a good idea.

## ⚠️ Known Limitation
If the computer is in hibernate/sleep, the calculated time will include the hibernate/sleep time. \
Doing a file checksum on the game binary seems to give bad result (probably because the game is executed?)

## 💡 Ideas / TODO
Fix [Known Limitation](https://github.com/EE-modders/Empire-Earth-Stats#%EF%B8%8F-known-limitation) \
Detect install way (Sierra, Community Setup, GOG, etc...) and report the setup version \
Create a config file to allow Computer ID to be anonymized and/or specific reports to be disabled (or disable entirely the DLL). \
Maybe stop using cURL to use boost with XP feature set to make the DLL working from XP :>

## ❤️ Credit
[zocker_160](https://github.com/zocker-160) \
cURL (License: https://github.com/curl/curl/blob/master/COPYING, Website: https://curl.se/) \
sha512.h (License: BSD, Author: Stefan Wilhelm), sha1.h (License: "public domain", Author: Steve Reid)

## 📖 License
[GNU General Public License v3.0](https://github.com/EE-modders/Empire-Earth-Launcher/blob/main/LICENSE)
