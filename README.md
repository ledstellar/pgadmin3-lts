** Fork info **

This is the fork of allentc/pgadmin3-lts repository intended specifically
to build pgAdmin3 on Ubuntu 22.04 (my current primary OS). 

I've created this fork only because I didn't manage to build pgAdmin
from allentc's repo. So I had to apply some minor changes to sources first.
As I am not a professional C coder I'm not sure these changes will not break
builds for other OSes though.

**# pgAdmin3 LTS by BigSQL README #**

pgAdmin3 has been superseded by pgAdmin4.  This repo exists to preserve
the work as it was left by BigSQL and consolidate whatever maintenance
effort occurs.  Binaries are not provided for any platform; you must build
the application yourself.

Introduction
------------

pgAdmin3 is a popular and feature rich Open Source administration and
development platform for PostgreSQL, the most advanced Open Source database in
the world.

pgAdmin3 is designed to answer the needs of all users, from writing simple 
SQL queries to developing complex databases. The graphical interface supports 
all PostgreSQL features and makes administration easy. The application also 
includes a syntax highlighting SQL editor, a server-side code editor, an 
SQL/batch/shell job scheduling agent, support for the Slony-I replication 
engine and much more. Server connection may be made using TCP/IP or Unix Domain
Sockets (on *nix platforms), and may be SSL encrypted for security. No 
additional drivers are required to communicate with the database server.

pgAdmin3 is Free Software released under the PostgreSQL License.

**# README by Datatrans #**

Code has been changed to adapt PostgreSQL internal changes up to version 15.2:
- No more relhasoids in pg_class.
- No more cache_value, is_cycled, is_called in sequence object (since PostgreSQL 11).
- No more adsrc in pg_attrdef, it should be calculated as pg_catalog.pg_get_expr(adbin, adrelid) instead.
- Declarative Table Partitioning DDL.
- No more datlastsysoid in pg_database.

Complete build sequence on a pristine Ubuntu 22.04:
------------------------
```

# PostgreSQL repo
sudo sh -c 'echo "deb http://apt.postgresql.org/pub/repos/apt $(lsb_release -cs)-pgdg main" > /etc/apt/sources.list.d/pgdg.list'
wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -
sudo apt-get update

# required packages (including PostgreSQL 15)
sudo apt install git postgresql-15 postgresql-server-dev-15 libwxgtk3.0-gtk3-dev wx3.0-headers libxslt1-dev libssh2-1-dev automake build-essential

# downloading sources into ~/projects/pgadmin3-lts
mkdir -p ~/projects
cd ~/projects
git clone https://github.com/ledstellar/pgadmin3-lts
cd pgadmin3-lts

# building
mkdir build
bash bootstrap
cd build
../configure --prefix=/opt/pgadmin3bigsql CFLAGS="-fPIC -g -O0" CXXFLAGS="-fPIC -DDEBUG" --with-pgsql=/usr/lib/postgresql/15 --without-sphinx-build
make -j$(nproc)
sudo make install

# run!
/opt/pgadmin3bigsql/bin/pgadmin3
```
