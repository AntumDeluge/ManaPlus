# Maintainer: Aline Ferraz <freya.df@gmail.com>

pkgname=manaplus-git
pkgver=1.1.6.12
pkgrel=1
pkgdesc="ManaPlus is a 2D MMORPG game advanced client for games based on eAthena fork The Mana World (tAthena) also for other forks like Evol. More info here: http://manaplus.evolonline.org"
arch=('i686' 'x86_64')
url="http://manaplus.evolonline.org"
depends=('libxml2' 'physfs' 'sdl_image' 'sdl_mixer' 'sdl_net' 'sdl_ttf' 'sdl_gfx')
license=('GPL2')

build() {
cd ${startdir}/
autoreconf -i
./configure --prefix=/usr
make clean
make || return 1
make DESTDIR=$startdir/pkg install
} 
