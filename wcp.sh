#!/usr/bin/env bash

PIPE_IN="/tmp/wcp"

# reset fifo in case it already exists and used by an older process
rm -f $PIPE_IN
mkfifo $PIPE_IN
exec 3<>$PIPE_IN

# update sliders and labels in ui
function update()
{
    # <- PULSEAUDIO
    vol=$(pulsemixer --get-volume | awk '{print $1;}')
    volrat=$(echo "scale=2 ; $vol / 100" | bc)
    # ->

    ################ set volume ####################
    echo "set ratio div volslider value $volrat" >&3
    ################################################
    
    # <- BRIGHTNESSCONTROL
    act=$(brightnessctl g)
    max=$(brightnessctl m)
    lcdrat=$(echo "scale=2 ; $act / $max" | bc)
    # ->

    ############### set brightness #################
    echo "set ratio div lcdslider value $lcdrat" >&3
    ################################################
    
    # <- BLUETOOTHCTL
    device=$(bluetoothctl info | sed -n 2p | awk '{for(i=2;i<=NF;i++) printf $i" "}')
    # ->
    if [ ${#device} -eq 0 ]
    then
	device="No_device_connected"
    fi

    ######### set bluetooth device's name ###########
    echo "set text div btoothlabel value $device" >&3
    #################################################
    
    # <- NMCLI
    network=$(nmcli con show | awk 'NR==2 {print $1}')
    # ->

    ############# set wifi network name ############
    echo "set text div wifilabel value $network" >&3
    ################################################
    
    printer=$(lpstat -e)

    ############### set default printer name #################
    echo "set text div printerlabel value ${printer// /_}" >&3
    ##########################################################
    
    layout=$(swaymsg -t get_inputs | jq '.[0].xkb_active_layout_name' | sed 's/^.//;s/.$//')

    ############### set keyoard laypout name #################
    echo "set text div keyboardlabel value ${layout// /_}" >&3
    ##########################################################
}

kuid -v <&3 | while IFS= read -r line; do                     # start kuid with file descriptor 3 as input and pipe its output to a reader

    echo $line
    words=($line)
    if [ ${words[0]} = "event" ]                              # analyze words in kuid output
    then

	if [ ${words[1]} = "init" ]                           # init event arrived
	then
	    ########################## init layer ##########################
	    echo "create layer width 300 height 258 anchor rt margin 10" >&3
	    echo "load html src wcp/main.html" >&3
	    ################################################################
	    
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

		swaymsg exec pavucontrol                      # pavucontrol

	    elif [ ${words[3]} == "displaybtn" ]              # display button pressed
	    then

		swaymsg exec wdisplays                        # wdisplays

	    elif [ ${words[3]} == "wifibtn" ] ||
		 [ ${words[3]} == "wifilabelback" ]           # wifi button pressed
	    then

		# swaymsg exec iwgtk                          # iwgtk
		swaymsg exec "foot nmtui"                     # nmtui
      
	    elif [ ${words[3]} == "bluetoothbtn" ] ||
		 [ ${words[3]} == "btoothlabelbck" ]          # bluetooth button pressed
	    then                                              

		swaymsg exec blueman-manager                  # for blueman manager

	    elif [ ${words[3]} == "lockbtn" ]                 # lock button pressed
	    then

		swaymsg exec swaylock                         # for swaylock
		# gnome-screensaver-command -l                # for gnome screensaver

	    elif [ ${words[3]} == "logoutbtn" ]               # logout button pressed
	    then

		sway exit                                     # for exiting sway
		# swaymsg exec loginctl terminate-user $USER  # for exiting with loginctl

	    elif [ ${words[3]} == "suspendbtn" ]              # suspend button pressed
	    then

		# sudo /sbin/zzz                              # for void linux, etc
		swaymsg exec systemctl suspend                # for systemd
	
	    elif [ ${words[3]} == "rebootbtn" ]               # reboot button pressed
	    then
		
		# sudo /sbin/reboot                           # for void linux, etc
		swaymsg exec systemctl reboot                 # for systemd

	    elif [ ${words[3]} == "shutdownbtn" ]             # shutdown button pressed
 	    then
		
		# sudo /sbin/poweroff                         # for void linux, etc
		swaymsg exec systemctl poweroff               # for systemd

	    fi
	fi
    fi
done
