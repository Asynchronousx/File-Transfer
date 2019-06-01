#!/bin/bash

#DIRCHECK
if [ ! -d Client  ] || [ ! -d Server ] || [ ! -d secure_networking ]; then
	exit 0;
fi

echo "Installing.."

#SETENV
if [ -d ~/.local/share/FileTransfer ]; then 
	rm -rf ~/.local/share/FileTransfer
	rm -rf ~/FileUpload
	rm -rf ~/FileDownload	
fi

mkdir ~/.local/share/FileTransfer
cp -r Client secure_networking Server ~/.local/share/FileTransfer/

#BUILDSOURCE
wd=""
cd ~/.local/share/FileTransfer/Client/ 
mkdir dwn 
make -s
wd=$PWD"/dwn/"
sed -i "s+PATHTODIR+$wd+g" FTClient.c
ln -s ~/.local/share/FileTransfer/Client/dwn ~/FileDownload 
cd ~/.local/share/FileTransfer/Server/ 
mkdir upl 
make -s
wd=$PWD"/upl/"
sed -i "s+PATHTODIR+$wd+g" FTServer.c
ln -s ~/.local/share/FileTransfer/Server/upl ~/FileUpload 

#ALIASING
if [ -f ~/.bashrc ]; then
	echo "alias ftclient=~/.local/share/FileTransfer/Client/FTClient" >> ~/.bashrc
	echo "alias ftserver=~/.local/share/FileTransfer/Server/FTServer" >> ~/.bashrc
fi

if [ -f ~/.zshrc ]; then
	echo "alias ftclient=~/.local/share/FileTransfer/Client/FTClient" >> ~/.zshrc
	echo "alias ftserver=~/.local/share/FileTransfer/Server/FTServer" >> ~/.zshrc
fi

cd ~/.local/share/FileTransfer/Server/ && make -s
cd ~/.local/share/FileTransfer/Client/ && make -s

echo "Done!"
echo "To begin your transfer, make sure to read the how-to-use on https://github.com/Asynchronousx/File-Transfer"
