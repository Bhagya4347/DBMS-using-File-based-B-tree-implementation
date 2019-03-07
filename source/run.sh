Start(){
    if [ "$1" = "SQL" ]; then
      gnome-terminal -e "bash -c \"./rundb.sh; exec bash\""
    else
        gnome-terminal -e "echo" "Please type 'Start SQL' to start Database 1.0 @synergystrikers"
    fi
}
