!/bin/bash
for((F=2;F<=8;F*=2))
do

for((K0=1;K0<=3;K0++))

do for((K1=1;K1<=3;K1++))

do for((K2=1;K2<=3;K2++))

do for((M=2;M<=8;M*=2))

do for((R=8;R<=128;R*=4))

do

chmod 755 procsim

./procsim -r $R -j $K0 -k $K1 -l $K2 -f $F -m $M <traces/mcf.100k.trace

done

done

done

done

done

done
