# ! /bin/sh
service_name="CacheServer.exe $1"
start_cmd="./${service_name}"
log_file="Restart.log"
# Start for the first time.
echo "=== Start ${service_name} ==="
${start_cmd}
while true
do
    proc_num=`ps -ef | grep $service_name | grep -v grep | wc -l`
    if [ ${proc_num} -eq 0 ]
    then
        echo "Service crashed! Restarting..."
        echo `date +%Y-%m-%d` `date +%H:%M:%S`  $service_name >>$log_file
        ${start_cmd}
    fi
    sleep 1
done