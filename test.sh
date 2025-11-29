#!/bin/bash

NTEST=$1

if [ $NTEST -lt 1 ]; then
  NTEST=1
fi


for((i=0;i<$NTEST;i++))
do
  buf=$((RANDOM % (99999 - 7 + 1) + 7))
  TMP1=$(mktemp)
  TMP2=$(mktemp)
  TMPP=$(mktemp)
  TMPR=$(mktemp)
  ./testgen.sh gen 2000 250000 $TMP1 &>/dev/null
  ./testgen.sh poke $TMP1 $TMP2 &>/dev/null
  ./sbsdiff $TMP1 $TMP2 $TMPP
  ./sbsp $TMP1 $TMPP $TMPR $buf
  cmp -s $TMPR $TMP2
  if [ $? -ne 0 ]; then
    echo -e "TEST $i [buf=$buf] \tFAIL"
  else
    echo -e "TEST $i [buf=$buf] \tPASS"
  fi
  rm -f $TMP1 $TMP2 $TMPP $TMPR
done
