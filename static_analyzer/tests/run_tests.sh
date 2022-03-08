error=""

find . -name "*.nut" > ~nuts.tmp
find . -name "*.nut.txt" >> ~nuts.tmp
find ./expected_compiler_error -name "*.txt" >> ~nuts.tmp

sq3_static_analyzer \
  "--csq-exe:sq -m" \
  --files:~nuts.tmp --output:analysis_log.txt

if [ $? -ne 0 ]
then
  error="BASE TEST"
else
  sq3_static_analyzer \
    "--csq-exe:sq -m" \
    --predefinition-files:~nuts.tmp --files:~nuts.tmp --output:analysis_log.txt

  if [ $? -ne 0 ]
  then
    error="2 PASS SQ"
  fi
fi


rm ~nuts.tmp

if [[ $error == "" ]]
then
  echo "OK"
  exit 0
else
  echo "FAIL:" $error
  exit 1
fi
