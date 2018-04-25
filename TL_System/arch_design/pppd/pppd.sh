
sleep 35
ifconfig eth0 down
sleep 25

sync_flag=0

sync_time() {
    
    if [ $sync_flag == 0 ]
    then
        if [ -x /home/bin/netclient ];then
            /home/bin/netclient 
            if [ "$?" == "0" ];then
                sync_flag=`expr $sync_flag + 1`
            fi
        fi
    fi

}


reboot_loop_cnt=0

while true
do
    ping -s 1 -c 1 www.baidu.com
    if [ $? -eq 0 ]
    then
            reboot_loop_cnt=0
            sync_time
    else
            sleep 1
            ping -s 1 -c 2 www.163.com
            if [ $? -eq 0 ]
            then
                reboot_loop_cnt=0
                sync_time
            else
                reboot_loop_cnt=`expr $reboot_loop_cnt + 1`
                if [ $reboot_loop_cnt -ge 10 ]
                then
                    reboot_loop_cnt=0
                    reboot -h now
                    sleep 20
                fi

                killall pppd 
                sleep 20
                pppd call wcdma-VT1916 &
                sleep 20
            fi
    fi

    sleep 120
done




