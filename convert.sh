printf '%s ' ${2%.*}.d > ${2%.*}.d
printf '$(REV_%s_o) ' ${2%.*} | tr \. _ >> ${2%.*}.d
$1 -MM -MG -isystem cringe $2 >> ${2%.*}.d
