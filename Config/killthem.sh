#!/bin/sh

NAME=Master.exe
PROCESS=`ps -ef|grep $NAME|grep -v grep|grep -v PPID|awk '{ print $2}'`
for i in $PROCESS
do
  sudo kill -9 $i
done

NAME=CacheServer.exe
PROCESS=`ps -ef|grep $NAME|grep -v grep|grep -v PPID|awk '{ print $2}'`
for i in $PROCESS
do
  kill -9 $i
done

NAME=Client.exe
PROCESS=`ps -ef|grep $NAME|grep -v grep|grep -v PPID|awk '{ print $2}'`
for i in $PROCESS
do
  kill -9 $i
done

echo "好耶！"