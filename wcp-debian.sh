#!/usr/bin/env bash

PIPE_IN="/tmp/wcp"

# reset fifo in case it already exists and used by an older process
rm -f $PIPE_IN
mkfifo $PIPE_IN
exec 3<>$PIPE_IN

# update sliders and labels in ui
function update()
{
    vol=$(pamixer --get-volume | awk '{print $1;}')
    volrat=$(echo "scale=2 ; $vol / 100" | bc)

    lcdact=$(brightnessctl g)
    lcdmax=$(brightnessctl m)
    lcdrat=$(echo "scale=2 ; $lcdact / $lcdmax" | bc)

    btdevice=$(bluetoothctl info | sed -n 2p | awk '{for(i=2;i<=NF;i++) printf $i" "}')
    if [ ${#btdevice} -eq 0 ]
    then
	btdevice="No_device_connected"
    fi

    network=$(iwctl station wlp0s20f3 show | sed -n 7p | awk '{print $NF}')

    printer=$(lpstat -e)

    layout=$(swaymsg -t get_inputs | jq '.[0].xkb_active_layout_name' | sed 's/^.//;s/.$//')

    echo "set ratio div volslider value $volrat" >&3
    echo "set ratio div lcdslider value $lcdrat" >&3
    echo "set text div btoothlabel value $btdevice" >&3
    echo "set text div wifilabel value $network" >&3
    echo "set text div printerlabel value ${printer// /_}" >&3
    echo "set text div keyboardlabel value ${layout// /_}" >&3
}

kuid -v <&3 | while IFS= read -r line; do                     # start kuid with file descriptor 3 as input and pipe its output to a reader

    echo $line
    words=($line)
    if [ ${words[0]} = "event" ]                              # analyze words in kuid output
    then

	if [ ${words[1]} = "init" ]                           # init event arrived
	then

	    echo "create layer width 300 height 258 anchor rt margin 10" >&3
	    echo "load html src ~/.config/wcp/res/main.html" >&3
	    
	    update                                            # update sliders and labels

	elif [ ${words[1]} = "update" ]
	then

	    update	                                      # update sliders and labels

	# slider events
	elif [ ${words[1]} == "ratio" ]
	then
	    if [ ${words[3]} == "volslider" ]                 # volume slider moved
	    then

		vol=${words[7]}
		res=$(pactl set-sink-volume @DEFAULT_SINK@ $vol%) # pactl

	    elif [ ${words[3]} == "lcdslider" ]               # lcd slider moved
	    then

		lcd=${words[7]}
		res=$(brightnessctl set $lcd%)                # brightnessctl

	    fi

	# button events
	elif [ ${words[1]} == "state" ]
	then
	    if [ ${words[3]} == "mutebtn" ]                   # volume button pressed
	    then

		swaymsg exec pavucontrol

	    elif [ ${words[3]} == "displaybtn" ]              # display button pressed
	    then

		swaymsg exec wdisplays

	    elif [ ${words[3]} == "wifibtn" ] ||
		 [ ${words[3]} == "wifilabelback" ]           # wifi button pressed
	    then

		swaymsg exec iwgtk
      
	    elif [ ${words[3]} == "bluetoothbtn" ] ||
		 [ ${words[3]} == "btoothlabelbck" ]          # bluetooth button pressed
	    then                                              

		swaymsg exec blueman-manager

	    elif [ ${words[3]} == "lockbtn" ]                 # lock button pressed
	    then

		swaymsg exec swaylock

	    elif [ ${words[3]} == "logoutbtn" ]               # logout button pressed
	    then

		sway exit

	    elif [ ${words[3]} == "suspendbtn" ]              # suspend button pressed
	    then

		swaymsg exec systemctl suspend
	
	    elif [ ${words[3]} == "rebootbtn" ]               # reboot button pressed
	    then

		swaymsg exec systemctl reboot

	    elif [ ${words[3]} == "shutdownbtn" ]             # shutdown button pressed
 	    then

		swaymsg exec systemctl poweroff

	    fi
	fi
    fi
done
