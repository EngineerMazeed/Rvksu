#!/bin/sh

LOCK_FILE="/tmp/isrvksu_login.lock"

IsRvksu="Rvksu@test"
IsRvksuPass="Rvksu@test.ca"

IsRvksuMgr="Admin@Rvksu"
IsRvksuMgrPass="Rvksu@Test"

is_login=0
is_root=0
count=0

trap 'stty echo; echo; exit' INT QUIT

echo " _____        _              "
echo "|  __ \      | |             "
echo "| |__) |_   _| | _____ _   _ "
echo "|  _  /\ \ / / |/ / __| | | |"
echo "| | \ \ \ V /|   <\__ \ |_| |"
echo "|_|  \_\ \_/ |_|\_\___/\__,_|"
echo ""
echo "=============================================================="
echo ""
echo " WARNING: If not authorized to access this system,"
echo " disconnect now."
echo ""
echo " YOU SHOULD HAVE NO EXPECTATION OF PRIVACY."
echo ""
echo " By continuing, you consent to your keystrokes"
echo " and data content being monitored."
echo ""
echo " Access is restricted to authorized users only."
echo ""
echo " Unauthorized access is a violation of state and"
echo " federal, civil and criminal laws."
echo ""
echo " Violations will be prosecuted to the fullest"
echo " extent of the law."
echo ""
echo "=============================================================="
echo ""

if [ -f "$LOCK_FILE" ]; then
    lock_time=$(cat "$LOCK_FILE")
    current_time=$(date +%s)

    if [ "$current_time" -lt "$lock_time" ]; then
        remain=$((lock_time - current_time))
        echo "Login locked. Try again in $remain seconds."
        exit 1
    else
        rm -f "$LOCK_FILE"
    fi
fi

while [ "$is_login" = "0" ]
do
    echo -n "Login: "
    read user

    echo -n "Password: "
    stty -echo
    read passwd
    stty echo
    echo

    if [ "$user" = "$IsRvksu" ] && \
       [ "$passwd" = "$IsRvksuPass" ]; then

        is_login=1
        is_root=1

    elif [ "$user" = "$IsRvksuMgr" ] && \
         [ "$passwd" = "$IsRvksuMgrPass" ]; then

        is_login=1
        is_root=0

    else
        echo "Incorrect login"
        count=$((count + 1))
    fi

    if [ "$count" -ge 3 ]; then
        echo "Too many failed attempts"
        echo "Locked for 5 minutes"

        future_time=$(( $(date +%s) + 300 ))
        echo "$future_time" > "$LOCK_FILE"

        exit 1
    fi
done

while [ "$is_login" = "1" ] && [ "$is_root" = "0" ]
do
    path=`pwd`
    echo -n "$path $ "
    read cmd

    case "$cmd" in
        *";"*|*"&"*|*"|"*|*"\`"*|*"$("*|*")"*|*">"*|*"<"*)
            echo "Blocked"
            continue
        ;;
    esac

    case "$cmd" in

        ls)
            /bin/ls
        ;;

        pwd)
            /bin/pwd
        ;;

        ps)
            /bin/ps
        ;;

        top)
            top
        ;;

        free)
            free
        ;;

        uptime)
            uptime
        ;;

        date)
            /bin/date
        ;;

        reboot)
            /sbin/reboot
        ;;

        "cat /proc/meminfo")
            /bin/cat /proc/meminfo
        ;;

        "cat /proc/cpuinfo")
            /bin/cat /proc/cpuinfo
        ;;

        exit)
            exit 0
        ;;

        "")
            continue
        ;;

        *)
            echo "operation failed: Permission denied"
        ;;
    esac
done

if [ "$is_root" = "1" ]; then
    /bin/sh
fi
