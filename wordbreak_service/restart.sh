export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/thread_pool/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/log4cplus/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yangrq/cheetah/src/wordbreak/lib

FILE_PATH=`cd $(dirname $0); pwd`
LOG_PATH=$FILE_PATH/../log

killall wordbreak_service -9

if [ ! -d $LOG_PATH ]
then
    echo 'mkdir ' $LOG_PATH
    mkdir $LOG_PATH
fi

rm -f ../log/*

./wordbreak_service -d
