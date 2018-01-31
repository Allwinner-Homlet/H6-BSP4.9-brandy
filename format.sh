#!/bin/sh
FUCK=$(git diff HEAD~1|grep diff|awk '{print $NF}'|sed -s "s/b\///"|xargs)
echo $FUCK
chmod a-x $FUCK
sed -s -i 's/[[:space:]]*$//' $FUCK
#zip  rls.zip -ur $FUCK
