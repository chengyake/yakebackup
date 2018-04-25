times=0
while true
do

    ./a.out 100000
    let times++
    echo  "exec $times x 10000 times"
    sleep 1

done
