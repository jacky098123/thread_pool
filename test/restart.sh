export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/thread_pool/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/log4cplus/lib

FILE_PATH=`cd $(dirname $0); pwd`
LOG_PATH=$FILE_PATH/../log

killall test_thread_pool -9

if [ ! -d $LOG_PATH ]
then
    echo 'mkdir ' $LOG_PATH
    mkdir $LOG_PATH
fi

rm -f ../log/*

./test_thread_pool
